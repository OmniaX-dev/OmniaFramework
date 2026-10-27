/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

	This file is part of OmniaFramework.

	OmniaFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	OmniaFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with OmniaFramework.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "BasicRenderer.hpp"
#include "RenderContext.hpp"
#include "../resources/EmbeddedShaders.hpp"
#include "../gui/Window.hpp"
#include "../resources/UbuntuTTF.hpp"
#include "../gui/Window.hpp"

#define COLOR_CAST(ostd_color) std::bit_cast<SDL_FColor>(ostd_color.getNormalizedColor())
#define QUAD_INDICES_ARR { 0, 1, 2, 2, 3, 0 }

namespace
{
	// Matches the BlurParams cbuffer in shaders/blur.frag.hlsl (std140 packing).
	struct BlurUniforms
	{
		f32 texelSize[2];
		f32 direction[2];
		f32 sigma;
		i32 radius;
		f32 padding[2];
	};
	static_assert(sizeof(BlurUniforms) == 32);

	// Width, in pixels, of the alpha-faded band generated around a flat-colored shape's outer
	// edge (see BasicRenderer2D::MeshProcessor below) - this is what turns a hard-edged quad/fan
	// into something that reads as anti-aliased without needing MSAA on the render target.
	constexpr f32 AA_FRINGE_WIDTH = 1.0f;
	// Vertices closer than 1/WELD_PRECISION pixels are merged before looking for the silhouette.
	constexpr f32 WELD_PRECISION = 64.0f;

	inline u64 weld_key(const ostd::Vec2& p)
	{
		const i32 qx = cast<i32>(std::lround(p.x * WELD_PRECISION));
		const i32 qy = cast<i32>(std::lround(p.y * WELD_PRECISION));
		return (cast<u64>(cast<u32>(qx)) << 32) | cast<u64>(cast<u32>(qy));
	}

	inline u64 edge_key(u32 a, u32 b)
	{
		return a < b ? ((cast<u64>(a) << 32) | b) : ((cast<u64>(b) << 32) | a);
	}

	inline i32 circle_segments(f32 radius)
	{
		return std::clamp(cast<i32>(radius * 1.5f), 16, 1024);
	}

	inline i32 corner_segments(f32 radius)
	{
		if (radius <= 0.0f)
			return 0;
		return std::clamp(cast<i32>(std::ceil(radius)) + 2, 3, 128);
	}

	// Appends the outline of a rounded rectangle, clockwise (y down), starting at the top-left
	// corner. radii: TL, TR, BR, BL. Each corner contributes segs[i] + 1 points (a single point
	// when segs[i] == 0), so two contours built with the same segs correspond point by point.
	void round_rect_contour(stdvec<ostd::Vec2>& out, const ostd::Rectangle& b, const f32 radii[4], const i32 segs[4])
	{
		const ostd::Vec2 centers[4] = {
			{ b.x + radii[0],       b.y + radii[0] },
			{ b.x + b.w - radii[1], b.y + radii[1] },
			{ b.x + b.w - radii[2], b.y + b.h - radii[2] },
			{ b.x + radii[3],       b.y + b.h - radii[3] }
		};
		const f32 startAngles[4] = { cast<f32>(M_PI), cast<f32>(M_PI * 1.5), 0.0f, cast<f32>(M_PI * 0.5) };
		for (i32 c = 0; c < 4; c++)
		{
			if (segs[c] <= 0)
			{
				out.push_back(centers[c]);
				continue;
			}
			for (i32 i = 0; i <= segs[c]; i++)
			{
				const f32 angle = startAngles[c] + (cast<f32>(i) / cast<f32>(segs[c])) * cast<f32>(M_PI * 0.5);
				out.push_back({ centers[c].x + std::cos(angle) * radii[c], centers[c].y + std::sin(angle) * radii[c] });
			}
		}
	}
}

namespace ogfx
{
	// ===================================================== MESH PROCESSOR =====================================================
	// Turns a FlatMesh (arbitrary triangle soup, possibly built from several pieces) into the
	// final vertex/index arrays for a flat-colored draw:
	//   1. welds vertices that share a position, so pieces built separately (round-rect corners
	//      and edges, line caps, the closing vertex of a full circle) become one connected mesh;
	//   2. drops triangles that became degenerate after welding;
	//   3. finds the silhouette - edges used by exactly one triangle, looked up through a hash map
	//      keyed on the (min, max) index pair, so it is O(E) - and extrudes it outward by
	//      AA_FRINGE_WIDTH into a band that fades to transparent. Extruded vertices are shared
	//      between adjacent silhouette edges (mitered, with a 2x limit), so the band has no gaps
	//      at corners and internal seams never get a fringe.
	// Scratch buffers are kept between calls to avoid per-shape allocations.
	struct BasicRenderer2D::MeshProcessor
	{
		// Open-addressing (linear probing) u64 -> u32 table on flat arrays, reused between calls.
		// Used both to weld vertices (quantized position -> vertex) and to count edge uses
		// ((min, max) index pair -> count), so both steps are O(n).
		struct ProbeTable
		{
			inline static constexpr u32 Empty { 0xFFFFFFFF };
			stdvec<u64> keys;
			stdvec<u32> values;
			u32 mask { 0 };

			inline void reset(u32 expected)
			{
				u32 capacity = 16;
				while (capacity < expected * 2)
					capacity <<= 1;
				keys.resize(capacity);
				values.assign(capacity, Empty);
				mask = capacity - 1;
			}

			inline u32 slot(u64 key) const
			{
				u32 h = cast<u32>((key * 0x9E3779B97F4A7C15ull) >> 32) & mask;
				while (values[h] != Empty && keys[h] != key)
					h = (h + 1) & mask;
				return h;
			}
		};

		struct BoundaryEdge
		{
			u32 a, b;
			Vec2 normal;
		};

		stdvec<Vec2> positions;
		stdvec<SDL_FColor> colors;
		stdvec<u32> remap;
		stdvec<u32> triangles;
		stdvec<BoundaryEdge> boundary;
		stdvec<Vec2> vertexNormals;
		stdvec<Vec2> firstNormals;
		stdvec<f32> minDots;
		stdvec<i32> outIndex;
		stdvec<i32> fringeIndex;
		ProbeTable weldTable;
		ProbeTable edgeTable;

		void process(const FlatMesh& mesh, const Vec2& uv, bool antialias, stdvec<SDL_Vertex>& outVerts, stdvec<i32>& outInds)
		{
			outVerts.clear();
			outInds.clear();
			const u32 inCount = cast<u32>(mesh.positions.size());
			if (inCount == 0 || mesh.indices.size() < 3)
				return;

			weld(mesh, inCount);
			collect_triangles(mesh);
			if (triangles.empty())
				return;

			const u32 vertCount = cast<u32>(positions.size());
			outIndex.assign(vertCount, -1);
			outInds.reserve(triangles.size() * 3);
			for (u32 v : triangles)
			{
				if (outIndex[v] < 0)
				{
					outIndex[v] = cast<i32>(outVerts.size());
					outVerts.push_back({ { positions[v].x, positions[v].y }, colors[v], { uv.x, uv.y } });
				}
				outInds.push_back(outIndex[v]);
			}

			if (!antialias)
				return;

			collect_boundary(vertCount);
			if (boundary.empty())
				return;

			fringeIndex.assign(vertCount, -1);
			for (const auto& edge : boundary)
			{
				add_fringe_vertex(edge.a, uv, outVerts);
				add_fringe_vertex(edge.b, uv, outVerts);
			}
			for (const auto& edge : boundary)
			{
				const i32 a = outIndex[edge.a], b = outIndex[edge.b];
				const i32 ea = fringeIndex[edge.a], eb = fringeIndex[edge.b];
				outInds.push_back(a); outInds.push_back(b); outInds.push_back(eb);
				outInds.push_back(a); outInds.push_back(eb); outInds.push_back(ea);
			}
		}

		private:
			void weld(const FlatMesh& mesh, u32 inCount)
			{
				positions.clear();
				colors.clear();
				remap.resize(inCount);
				weldTable.reset(inCount);
				for (u32 i = 0; i < inCount; i++)
				{
					const u64 key = weld_key(mesh.positions[i]);
					const u32 h = weldTable.slot(key);
					if (weldTable.values[h] == ProbeTable::Empty)
					{
						weldTable.keys[h] = key;
						weldTable.values[h] = cast<u32>(positions.size());
						positions.push_back(mesh.positions[i]);
						colors.push_back(mesh.colors[i]);
					}
					remap[i] = weldTable.values[h];
				}
			}

			void collect_triangles(const FlatMesh& mesh)
			{
				triangles.clear();
				const u32 inCount = cast<u32>(remap.size());
				for (size_t t = 0; t + 2 < mesh.indices.size(); t += 3)
				{
					const u32 i0 = mesh.indices[t], i1 = mesh.indices[t + 1], i2 = mesh.indices[t + 2];
					if (i0 >= inCount || i1 >= inCount || i2 >= inCount)
						continue;
					const u32 a = remap[i0], b = remap[i1], c = remap[i2];
					if (a == b || b == c || c == a)
						continue;
					if (std::abs((positions[b] - positions[a]).cross(positions[c] - positions[a])) < 1e-6f)
						continue;
					triangles.push_back(a);
					triangles.push_back(b);
					triangles.push_back(c);
				}
			}

			void collect_boundary(u32 vertCount)
			{
				boundary.clear();
				edgeTable.reset(cast<u32>(triangles.size()));
				for (size_t t = 0; t < triangles.size(); t += 3)
				{
					for (i32 e = 0; e < 3; e++)
					{
						const u64 key = edge_key(triangles[t + e], triangles[t + (e + 1) % 3]);
						const u32 h = edgeTable.slot(key);
						if (edgeTable.values[h] == ProbeTable::Empty)
						{
							edgeTable.keys[h] = key;
							edgeTable.values[h] = 1;
						}
						else
							edgeTable.values[h]++;
					}
				}
				vertexNormals.assign(vertCount, { 0.0f, 0.0f });
				firstNormals.assign(vertCount, { 0.0f, 0.0f });
				minDots.assign(vertCount, 1.0f);
				for (size_t t = 0; t < triangles.size(); t += 3)
				{
					for (i32 e = 0; e < 3; e++)
					{
						const u32 a = triangles[t + e];
						const u32 b = triangles[t + (e + 1) % 3];
						const u32 c = triangles[t + (e + 2) % 3];
						if (edgeTable.values[edgeTable.slot(edge_key(a, b))] != 1)
							continue;
						const Vec2 d = positions[b] - positions[a];
						const f32 len = d.mag();
						if (len < 1e-6f)
							continue;
						Vec2 n { d.y / len, -d.x / len };
						if (n.dot(positions[c] - positions[a]) > 0.0f)
							n = n * -1.0f;
						boundary.push_back({ a, b, n });
						for (u32 v : { a, b })
						{
							if (vertexNormals[v].x == 0.0f && vertexNormals[v].y == 0.0f)
								firstNormals[v] = n;
							vertexNormals[v] += n;
						}
					}
				}
				for (const auto& edge : boundary)
				{
					for (u32 v : { edge.a, edge.b })
					{
						Vec2 n = vertexNormals[v];
						if (n.mag() < 1e-4f)
							continue;
						n.normalizem();
						minDots[v] = std::min(minDots[v], n.dot(edge.normal));
					}
				}
			}

			void add_fringe_vertex(u32 v, const Vec2& uv, stdvec<SDL_Vertex>& outVerts)
			{
				if (fringeIndex[v] >= 0)
					return;
				Vec2 n = vertexNormals[v];
				if (n.mag() < 1e-4f)
					n = firstNormals[v];
				else
					n.normalizem();
				const f32 dist = AA_FRINGE_WIDTH / std::max(minDots[v], 0.5f);
				SDL_FColor faded = colors[v];
				faded.a = 0.0f;
				fringeIndex[v] = cast<i32>(outVerts.size());
				outVerts.push_back({ { positions[v].x + n.x * dist, positions[v].y + n.y * dist }, faded, { uv.x, uv.y } });
			}
	};
	// ===================================================== MESH PROCESSOR =====================================================




	// ===================================================== SIGNAL HANDLER =====================================================
	BasicRenderer2D::SignalHandler::SignalHandler(BasicRenderer2D& parent) : m_parent(parent)
	{
		enableSignals();
		connectSignal(ostd::BuiltinSignals::BeforeSDLShutdown);
		setTypeName("ostd::GraphicsWindowOutputHandler::SignalHandler");
		validate();
	}

	void BasicRenderer2D::SignalHandler::handleSignal(ostd::Signal& signal)
	{
		if (signal.ID == ostd::BuiltinSignals::BeforeSDLShutdown)
		{
			m_parent.closeFont();
			m_parent.close_mono_font();
		}
	}
	// ===================================================== SIGNAL HANDLER =====================================================





	// ===================================================== UTILS =====================================================
	BasicRenderer2D::BasicRenderer2D(void) : m_meshProcessor(std::make_unique<MeshProcessor>())
	{
	}

	BasicRenderer2D::~BasicRenderer2D(void)
	{
		// Internal targets/shaders unregister themselves, so they go while everything is intact.
		m_blurCache.clear();
		m_blurShaderH.reset();
		m_blurShaderV.reset();
		if (m_initialized)
			__release_device_resources();
		closeFont();
		close_mono_font();
		for (auto* target : stdvec<RenderTarget*>(m_targets))
			target->detach_from_renderer();
		for (auto* image : stdvec<Image*>(m_images))
			image->__detach_from_renderer();
		for (auto* shader : stdvec<Shader*>(m_shaders))
			shader->release_renderer(*this);
		RenderContext::get().unregisterRenderer(*this);
	}

	i32 BasicRenderer2D::init(WindowCore& window)
	{
		m_window = &window;
		if (m_initialized) return set_error_state(tErrors::NoError);
		RenderContext::get().registerRenderer(*this);
		SDL_SetRenderDrawBlendMode(m_window->getSDLRenderer(), SDL_BLENDMODE_BLEND);
		m_initialized = true;
		loadDefaultFont(20);
		m_initialized = false;
		m_fontGlyphAtlas.init(*this);
		auto atlas = m_fontGlyphAtlas.getAtlas(0);
		if (atlas) //TODO: Error
			m_lastUsedGlyphAtlasTex = atlas;
		m_initialized = true;
		return set_error_state(tErrors::NoError);
	}

	SDL_Renderer* BasicRenderer2D::getSDLRenderer(void) const
	{
		return m_window != nullptr ? m_window->getSDLRenderer() : nullptr;
	}

	SDL_WindowID BasicRenderer2D::getWindowID(void) const
	{
		return (m_window != nullptr && m_window->getSDLWindow() != nullptr) ? SDL_GetWindowID(m_window->getSDLWindow()) : 0;
	}

	void BasicRenderer2D::flushBatch(void)
	{
		if (!m_initialized || m_vertexCount == 0)
			return;
		SDL_Renderer* renderer = m_window->getSDLRenderer();
		// The blend mode of a geometry draw comes from its texture (or from the renderer's draw
		// blend mode when untextured), so it is applied right before submitting the batch.
		if (m_batchState.texture)
			SDL_SetTextureBlendMode(m_batchState.texture, m_batchState.blend);
		else
			SDL_SetRenderDrawBlendMode(renderer, m_batchState.blend);
		if (m_batchState.shader)
			SDL_SetGPURenderState(renderer, m_batchState.shader);
		SDL_RenderGeometry(renderer, m_batchState.texture, m_vertices.data(), m_vertexCount, m_indices.data(), m_indexCount);
		// Reset right away, so SDL draws made outside of this renderer never inherit the shader.
		if (m_batchState.shader)
			SDL_SetGPURenderState(renderer, nullptr);
		m_drawCallCount++;
		m_batchState = { };
		m_vertexCount = 0;
		m_indexCount = 0;
	}

	void BasicRenderer2D::endFrame(void)
	{
		if (!m_targetStack.empty())
		{
			OX_ERROR("BasicRenderer2D::endFrame() called with %d render target(s) still bound.", cast<i32>(m_targetStack.size()));
			unwind_render_targets();
		}
		if (!m_shaderStack.empty())
		{
			OX_ERROR("BasicRenderer2D::endFrame() called with %d shader(s) still active.", cast<i32>(m_shaderStack.size()));
			m_shaderStack.clear();
			update_active_shader();
		}
		flushBatch();
		m_frameIndex++;
		m_lastFrameDrawCallCount = m_drawCallCount;
		m_drawCallCount = 0;
	}

	void BasicRenderer2D::pushClippingRect(const Rectangle& rect, bool additive)
	{
		if (!m_initialized) return;

		Rectangle finalRect = rect;

		if (additive && !m_clipStack.empty())
			finalRect = m_clipStack.back().getIntersection(rect, false);

		if (!m_clipStack.empty() && m_clipStack.back() == finalRect)
		{
			m_clipStack.push_back(finalRect);
			return;
		}
		flushBatch();

		m_clipStack.push_back(finalRect);
		apply_clip_rect();
	}

	void BasicRenderer2D::popClippingRect(void)
	{
		if (!m_initialized) return;
		if (m_clipStack.empty()) return;

		Rectangle popped = m_clipStack.back();
		m_clipStack.pop_back();

		if (!m_clipStack.empty() && m_clipStack.back() == popped)
			return;

		flushBatch();
		apply_clip_rect();
	}

	i32 BasicRenderer2D::loadDefaultMonoFont(i32 fontSize)
	{
		return openFont(ubuntu_mono_regular_ttf_data, ubuntu_mono_regular_ttf_size, fontSize);
	}

	i32 BasicRenderer2D::loadDefaultFont(i32 fontSize)
	{
		return openFont(ubuntu_regular_ttf_data, ubuntu_regular_ttf_size, fontSize);
	}

	TTF_Font* BasicRenderer2D::getDefaultMonoFont(void)
	{
		if (m_monoFont != nullptr || !m_initialized)
			return m_monoFont;
		SDL_IOStream* rw = SDL_IOFromConstMem(ubuntu_mono_regular_ttf_data, ubuntu_mono_regular_ttf_size);
		m_monoFont = TTF_OpenFontIO(rw, true, cast<f32>(m_fontSize));
		if (m_monoFont == nullptr)
			print_ttf_error("TTF_OpenFontIO");
		return m_monoFont;
	}

	void BasicRenderer2D::pushFont(TTF_Font* font)
	{
		m_fontStack.push_back(m_font);
		if (font != nullptr)
			m_font = font;
	}

	void BasicRenderer2D::popFont(void)
	{
		if (m_fontStack.empty())
			return;
		m_font = m_fontStack.back();
		m_fontStack.pop_back();
	}

	void BasicRenderer2D::close_mono_font(void)
	{
		if (m_monoFont == nullptr)
			return;
		// A font still pushed must not be left dangling.
		for (auto& font : m_fontStack)
		{
			if (font == m_monoFont)
				font = nullptr;
		}
		if (m_font == m_monoFont)
			m_font = m_fontStack.empty() ? nullptr : m_fontStack.front();
		TTF_CloseFont(m_monoFont);
		m_monoFont = nullptr;
	}

	void BasicRenderer2D::closeFont(void)
	{
		if (!m_initialized || !m_fontOpen) return;
		if (m_font != nullptr)
		{
			TTF_CloseFont(m_font);
		}
		m_font = nullptr;
		m_fontOpen = false;
		m_kernCache.clear();
		m_fontSize = DefaultFontSize;
		m_fontFromMemory = false;
		set_error_state(tErrors::NoError);
	}

	i32 BasicRenderer2D::openFont(const String& fontPath, i32 fontSize)
	{
		if (!m_initialized) return set_error_state(tErrors::Uninitialized);
		if (m_fontOpen) closeFont();
		if (fontSize <= 0)
			fontSize = m_fontSize;
		else
			m_fontSize = fontSize;
		m_font = TTF_OpenFont(fontPath.c_str(), fontSize);
		if (m_font == nullptr)
		{
			print_ttf_error("TTF_OpenFont");
			return set_error_state(tErrors::FailedToOpenFontFile);
		}
		m_fontOpen = true;
		m_fontFromMemory = false;
		return set_error_state(tErrors::NoError);
	}

	i32 BasicRenderer2D::openFont(const ostd::UByte resource_data[], u32 data_size, i32 fontSize)
	{
		if (!m_initialized) return set_error_state(tErrors::Uninitialized);
		if (data_size < 100) // Arbitrary 100 bytes
			return set_error_state(tErrors::FailedToOpenFontFile);
		if (m_fontOpen) closeFont();
		if (fontSize <= 0)
			fontSize = m_fontSize;
		else
			m_fontSize = fontSize;
		SDL_IOStream* rw = SDL_IOFromConstMem(resource_data, data_size);
		m_font = TTF_OpenFontIO(rw, true, fontSize);
		if (m_font == nullptr)
		{
			print_ttf_error("TTF_OpenFont");
			return set_error_state(tErrors::FailedToOpenFontFile);
		}
		m_fontOpen = true;
		m_fontFromMemory = true;
		return set_error_state(tErrors::NoError);
	}

	i32 BasicRenderer2D::setFontSize(i32 fontSize)
	{
		if (!m_initialized)
			return set_error_state(tErrors::Uninitialized);
		if (!m_fontOpen)
			return set_error_state(tErrors::NoFont);
		if (m_font == nullptr)
			return set_error_state(tErrors::NullFont);
		if (fontSize == m_fontSize || fontSize <= 0)
			return set_error_state(tErrors::NoError);
		TTF_SetFontSize(m_font, fontSize);
		m_fontSize = fontSize;
		return set_error_state(tErrors::NoError);
	}

	Vec2 BasicRenderer2D::getStringDimensions(const String& message, i32 fontSize, TTF_Font* font)
	{
		auto dimensions = getStringDimensionsPerCharacter(message, fontSize, font);
		if (dimensions.empty())
			return { 0, 0 };
		f32 totalWidth = 0;
		for (const auto& d : dimensions)
			totalWidth += d.x;
		return { totalWidth, dimensions[0].y };
	}

	stdvec<Vec2> BasicRenderer2D::getStringDimensionsPerCharacter(const String& message, i32 fontSize, TTF_Font* font)
	{
		if (!isValid()) return {  };
		if (fontSize <= 0) fontSize = m_fontSize;
		if (!font) font = m_font;

		auto l_buildCacheKey = [&](void) -> String {
			return String("").add(message).add(fontSize).addPtr(font);
		};

		String cacheKey = l_buildCacheKey();

		auto it = m_strDimsCache.find(cacheKey);
		if (it != m_strDimsCache.end())
		{
			m_cacheHitCount++;
			return it->second;
		}

		// No font size switch here: the atlas and get_kerning() switch it only when they have
		// to ask SDL_ttf (switching flushes SDL_ttf's glyph cache).
		auto glyphs = m_fontGlyphAtlas.processString(message, font, fontSize);
		if (glyphs.empty()) return {  };

		stdvec<Vec2> dimensions;
		dimensions.reserve(message.len());

		f32 charWidth = 0;
		for (size_t i = 0; i < glyphs.size(); i++)
		{
			charWidth += glyphs[i]->advance;
			if (i > 0)
			{
				charWidth += get_kerning(font, fontSize, glyphs[i - 1]->codepoint, glyphs[i]->codepoint);
			}
			dimensions.push_back({ charWidth, glyphs[0]->size.y });
			charWidth = 0;
		}

		if (m_strDimsCache.size() >= MaxStringDimsCacheSize)
			m_strDimsCache.pop_back();
		m_strDimsCache.insert(cacheKey, dimensions);

		m_cacheMissCount++;
		return dimensions;
	}
	// ===================================================== UTILS =====================================================





	// ===================================================== SPECIALIZED =====================================================
	void BasicRenderer2D::drawImage(const ogfx::Image& image, const Vec2& position, const Vec2& size, const Color& tint, const Rectangle& srcRect)
	{
		if (!m_initialized || !image.isLoaded())
			return;

		SDL_Texture* tex = image.getSDLTexture();
		if (!tex)
			return;

		Vec2 texSize = image.getSize();

		// 1. Resolve source rectangle
		f32 sx, sy, sw, sh;
		if (srcRect.w == 0 && srcRect.h == 0)
		{
			// Use entire texture
			sx = 0.0f;
			sy = 0.0f;
			sw = texSize.x;
			sh = texSize.y;
		}
		else
		{
			sx = srcRect.x;
			sy = srcRect.y;
			sw = srcRect.w;
			sh = srcRect.h;
		}

		// 2. Resolve destination rectangle
		f32 dx = position.x;
		f32 dy = position.y;
		f32 dw = (size.x == 0) ? sw : size.x;
		f32 dh = (size.y == 0) ? sh : size.y;
		f32 x1 = dx;
		f32 y1 = dy;
		f32 x2 = dx + dw;
		f32 y2 = dy + dh;

		// 3. Build quad vertices
		Vec2 verts[4] = {
			{ x1, y1 },
			{ x2, y1 },
			{ x2, y2 },
			{ x1, y2 }
		};

		// 4. Build UVs (normalized)
		Vec2 uvs[4] = {
			{ sx / texSize.x,         sy / texSize.y },
			{ (sx + sw) / texSize.x,  sy / texSize.y },
			{ (sx + sw) / texSize.x, (sy + sh) / texSize.y },
			{ sx / texSize.x,        (sy + sh) / texSize.y }
		};

		// 5. Push quad
		emit_textured_quad(tex, verts, uvs, COLOR_CAST(tint));
	}

	void BasicRenderer2D::drawAnimation(const Animation& anim, const Vec2& position, const Vec2& size, const Color& tint)
	{
		if (!m_initialized) return;
		if (!anim.hasImage()) return;
		const Image& img = anim.getSpriteSheet();
		if (!img.isLoaded() || !img.isValid()) return;
		drawImage(img, position, size, tint, anim.getFrameRect());
	}

	void BasicRenderer2D::fillGradientRect(const Rectangle& rect, ColorGradient& gradient)
	{
		const auto& colors = gradient.getColors();
		stdvec<f32> weights = gradient.getWeights();
		f32 angle_radians = gradient.getAngleRad();
		if (!m_initialized || colors.size() < 2) return;
		if (weights.size() != colors.size() - 1) return; // N colors = N-1 weights

		// Normalize weights
		f32 total = 0;
		for (auto w : weights) total += w;
		if (total <= 0.0f) return;
		for (auto& w : weights) w /= total;

		// Compute cumulative positions along gradient (0 to 1)
		stdvec<f32> stops(colors.size());
		stops[0] = 0.0f;
		for (size_t i = 0; i < weights.size(); i++)
			stops[i + 1] = stops[i] + weights[i];

		// Center of rect for rotation
		Vec2 center = { rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f };
		f32 cs = std::cos(angle_radians);
		f32 sn = std::sin(angle_radians);

		auto rotate = [&](Vec2 p) -> Vec2 {
			p -= center;
			return { center.x + p.x * cs - p.y * sn,
					 center.y + p.x * sn + p.y * cs };
		};

		// One row of two vertices per stop; consecutive rows form the color bands, sharing their
		// vertices so the whole gradient is a single mesh with a fringe only on its outline.
		m_mesh.clear();
		for (size_t i = 0; i < colors.size(); i++)
		{
			f32 y = rect.y + rect.h * stops[i];
			SDL_FColor c = COLOR_CAST(colors[i]);
			m_mesh.addVertex(rotate({ rect.x, y }), c);
			m_mesh.addVertex(rotate({ rect.x + rect.w, y }), c);
		}
		for (u32 i = 0; i + 1 < colors.size(); i++)
		{
			u32 tl = i * 2, tr = i * 2 + 1, bl = i * 2 + 2, br = i * 2 + 3;
			m_mesh.addTriangle(tl, tr, br);
			m_mesh.addTriangle(br, bl, tl);
		}
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::drawString(const String& str, const Vec2& position, const Color& color, i32 fontSize, f32 scale)
	{
		if (!isValid()) return;
		if (fontSize <= 0)
			fontSize = m_fontSize;
		auto glyphs = m_fontGlyphAtlas.processString(str, m_font, fontSize);
		if (glyphs.empty()) return;

		f32 x = position.x;
		f32 y = position.y;


		for (size_t i = 0; i < glyphs.size(); i++)
		{
			auto& g = glyphs[i];

			if (i > 0)
			{
				x += (get_kerning(m_font, fontSize, glyphs[i - 1]->codepoint, g->codepoint) * scale);
			}

			Vec2 verts[4] = {
				{ x,                        y },
				{ x + g->size.x * scale,     y },
				{ x + g->size.x * scale,     y + g->size.y * scale },
				{ x,                        y + g->size.y * scale }
			};
			emit_textured_quad(g->atlas, verts, g->uvs, COLOR_CAST(color));
			m_lastUsedGlyphAtlasTex = g->atlas;
			x += (g->advance * scale);
		}
	}

	void BasicRenderer2D::drawCenteredString(const String& str, const Vec2& center, const Color& color, i32 fontSize, f32 scale)
	{
		auto dims = getStringDimensions(str, fontSize);
		drawString(str, { center.x - dims.x * 0.5f, center.y - dims.y * 0.5f }, color, fontSize, scale);
	}

	void BasicRenderer2D::drawCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize, f32 scale)
	{
		drawCenteredString(str, Vec2 { bounds.x + bounds.w * 0.5f, bounds.y + bounds.h * 0.5f }, color, fontSize, scale);
	}

	void BasicRenderer2D::drawVCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize, f32 scale)
	{
		auto dims = getStringDimensions(str, fontSize);
		drawString(str, { bounds.x, (bounds.y + bounds.h * 0.5f) - dims.y * 0.5f }, color, fontSize, scale);
	}

	void BasicRenderer2D::drawHCenteredString(const String& str, const Rectangle& bounds, const Color& color, i32 fontSize, f32 scale)
	{
		auto dims = getStringDimensions(str, fontSize);
		drawString(str, { (bounds.x + bounds.w * 0.5f) - dims.x * 0.5f, bounds.y }, color, fontSize, scale);
	}
	// ===================================================== SPECIALIZED =====================================================





	// ===================================================== PRIMITIVES =====================================================
	void BasicRenderer2D::drawLine(const FLine& line, const Color& color, i32 thickness, bool rounded)
	{
		if (!m_initialized || thickness <= 0) return;

		Vec2 p1 = line.p1.asVec2();
		Vec2 p2 = line.p2.asVec2();
		if ((p2 - p1).mag() < 1e-4f)
		{
			// A zero-length line has no direction: a rounded one is a dot, a square one is nothing.
			if (rounded)
				fillCircle(p1, thickness * 0.5f, color);
			return;
		}
		m_mesh.clear();
		mesh_line(m_mesh, p1, p2, cast<f32>(thickness), rounded && thickness >= 4, COLOR_CAST(color));
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::drawRect(const Rectangle& rect, const Color& color, i32 thickness, bool topEdge, bool rightEdge, bool bottomEdge, bool leftEdge)
	{
		if (!m_initialized || thickness <= 0)
			return;
		if (!topEdge && !rightEdge && !bottomEdge && !leftEdge)
			return;

		// The outline is the border ring of a 3x3 grid (outer edges = rect, inner edges = rect inset
		// by the thickness); each enabled edge contributes its row/column of cells. The cells share
		// the grid's vertices, so the outline is one connected mesh without overlapping corners.
		f32 t = std::min(cast<f32>(thickness), std::min(rect.w, rect.h) * 0.5f);
		const f32 xs[4] = { rect.x, rect.x + t, rect.x + rect.w - t, rect.x + rect.w };
		const f32 ys[4] = { rect.y, rect.y + t, rect.y + rect.h - t, rect.y + rect.h };
		SDL_FColor col = COLOR_CAST(color);

		m_mesh.clear();
		for (i32 y = 0; y < 4; y++)
			for (i32 x = 0; x < 4; x++)
				m_mesh.addVertex({ xs[x], ys[y] }, col);
		for (i32 cy = 0; cy < 3; cy++)
		{
			for (i32 cx = 0; cx < 3; cx++)
			{
				bool include = (cy == 0 && topEdge) || (cy == 2 && bottomEdge) || (cx == 0 && leftEdge) || (cx == 2 && rightEdge);
				if (!include || (cx == 1 && cy == 1))
					continue;
				u32 tl = cy * 4 + cx, tr = tl + 1, bl = tl + 4, br = bl + 1;
				m_mesh.addTriangle(tl, tr, br);
				m_mesh.addTriangle(br, bl, tl);
			}
		}
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::drawRect(const Vec2& center, const Vec2& size, const Color& color, i32 thickness, bool topEdge, bool rightEdge, bool bottomEdge, bool leftEdge)
	{
		drawRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, thickness, topEdge, rightEdge, bottomEdge, leftEdge);
	}

	void BasicRenderer2D::drawRoundRect(const Rectangle& rect, const Color& color, f32 radius, i32 thickness)
	{
		drawRoundRect(rect, color, { radius, radius, radius, radius }, thickness);
	}

	void BasicRenderer2D::drawRoundRect(const Vec2& center, const Vec2& size, const Color& color, f32 radius, i32 thickness)
	{
		drawRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, { radius, radius, radius, radius }, thickness);
	}

	void BasicRenderer2D::drawRoundRect(const Vec2& center, const Vec2& size, const Color& color, const Rectangle& radii, i32 thickness)
	{
		drawRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, radii, thickness);
	}

	void BasicRenderer2D::drawRoundRect(const Rectangle& rect, const Color& color, const Rectangle& radii, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
			return;

		// radii: x=TL, y=TR, w=BR, h=BL - the radius of the stroke's center line, as before.
		f32 maxR = std::min(rect.w, rect.h) * 0.5f;
		f32 t = std::min(cast<f32>(thickness), maxR);
		f32 half = t * 0.5f;
		const f32 centerRadii[4] = { radii.x, radii.y, radii.w, radii.h };

		// The stroke is the band between an outer contour (the rect itself) and an inner contour
		// (the rect inset by the thickness). Both use the same number of points per corner, so they
		// can be stitched into a closed strip.
		f32 outerRadii[4], innerRadii[4];
		i32 segs[4];
		for (i32 i = 0; i < 4; i++)
		{
			f32 r = std::clamp(centerRadii[i], 0.0f, maxR);
			outerRadii[i] = (r > 0.0f ? std::min(r + half, maxR) : 0.0f);
			innerRadii[i] = std::max(0.0f, outerRadii[i] - t);
			segs[i] = corner_segments(outerRadii[i]);
		}
		Rectangle inner { rect.x + t, rect.y + t, std::max(0.0f, rect.w - t * 2.0f), std::max(0.0f, rect.h - t * 2.0f) };
		f32 innerMaxR = std::min(inner.w, inner.h) * 0.5f;
		for (i32 i = 0; i < 4; i++)
			innerRadii[i] = std::min(innerRadii[i], innerMaxR);

		auto& outerPts = m_contourA;
		auto& innerPts = m_contourB;
		outerPts.clear();
		innerPts.clear();
		round_rect_contour(outerPts, rect, outerRadii, segs);
		round_rect_contour(innerPts, inner, innerRadii, segs);

		SDL_FColor col = COLOR_CAST(color);
		m_mesh.clear();
		const u32 n = cast<u32>(outerPts.size());
		for (u32 i = 0; i < n; i++)
		{
			m_mesh.addVertex(outerPts[i], col);
			m_mesh.addVertex(innerPts[i], col);
		}
		for (u32 i = 0; i < n; i++)
		{
			u32 j = (i + 1) % n;
			u32 o0 = i * 2, i0 = i * 2 + 1, o1 = j * 2, i1 = j * 2 + 1;
			m_mesh.addTriangle(o0, o1, i0);
			m_mesh.addTriangle(o1, i1, i0);
		}
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::drawCircle(const Vec2& center, f32 radius, const Color& color, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
			return;
		m_mesh.clear();
		mesh_arc_stroke(m_mesh, center, radius, radius, cast<f32>(thickness), 0.0f, 2.0f * M_PI, circle_segments(radius), COLOR_CAST(color));
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::drawCircle(const Rectangle& rect, const Color& color, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radius = std::min(rect.w, rect.h) * 0.5f;
		drawCircle(center, radius, color, thickness);
	}

	void BasicRenderer2D::drawEllipse(const Rectangle& rect, const Color& color, i32 thickness)
	{
		if (!m_initialized || thickness <= 0)
		  return;

		Vec2 center = { rect.x + rect.w*0.5f, rect.y + rect.h*0.5f };
		f32 rx = rect.w * 0.5f;
		f32 ry = rect.h * 0.5f;
		m_mesh.clear();
		mesh_arc_stroke(m_mesh, center, rx, ry, cast<f32>(thickness), 0.0f, 2.0f * M_PI, circle_segments(std::max(rx, ry)), COLOR_CAST(color));
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::drawTriangle(const Triangle& tri, const Color& color, i32 thickness)
	{
		if (!m_initialized)
			return;
		drawTriangle(tri.vA, tri.vB, tri.vC, color, thickness);
	}

	void BasicRenderer2D::drawTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& color, i32 thickness)
	{
		if (!m_initialized)
			return;
		drawLine({ A, B }, color, thickness, false);
		drawLine({ B, C }, color, thickness, false);
		drawLine({ C, A }, color, thickness, false);
	}




	void BasicRenderer2D::fillRect(const Rectangle& rect, const Color& color)
	{
		if (!m_initialized)
			return;

		f32 x1 = rect.x;
		f32 y1 = rect.y;
		f32 x2 = rect.x + rect.w;
		f32 y2 = rect.y + rect.h;

		SDL_FColor col = COLOR_CAST(color);
		m_mesh.clear();
		m_mesh.addVertex({ x1, y1 }, col);
		m_mesh.addVertex({ x2, y1 }, col);
		m_mesh.addVertex({ x2, y2 }, col);
		m_mesh.addVertex({ x1, y2 }, col);
		m_mesh.addTriangle(0, 1, 2);
		m_mesh.addTriangle(2, 3, 0);
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::fillRect(const Vec2& center, const Vec2& size, const Color& color)
	{
		fillRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color);
	}

	void BasicRenderer2D::fillRoundRect(const Rectangle& rect, const Color& color, f32 radius)
	{
		fillRoundRect(rect, color, { radius, radius, radius, radius });
	}

	void BasicRenderer2D::fillRoundRect(const Vec2& center, const Vec2& size, const Color& color, f32 radius)
	{
		fillRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, { radius, radius, radius, radius });
	}

	void BasicRenderer2D::fillRoundRect(const Vec2& center, const Vec2& size, const Color& color, const Rectangle& radii)
	{
		fillRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, color, radii);
	}

	void BasicRenderer2D::fillRoundRect(const Rectangle& rect, const Color& color, const Rectangle& radii)
	{
		if (!m_initialized)
			return;

		// radii: x=TL, y=TR, w=BR, h=BL. A rounded rect is convex, so its outline triangulated as a
		// fan from the center is a single mesh - no internal seams for the AA fringe to pick up.
		f32 maxR = std::min(rect.w, rect.h) * 0.5f;
		const f32 r[4] = {
			std::clamp(radii.x, 0.0f, maxR),
			std::clamp(radii.y, 0.0f, maxR),
			std::clamp(radii.w, 0.0f, maxR),
			std::clamp(radii.h, 0.0f, maxR)
		};
		const i32 segs[4] = { corner_segments(r[0]), corner_segments(r[1]), corner_segments(r[2]), corner_segments(r[3]) };

		auto& contour = m_contourA;
		contour.clear();
		round_rect_contour(contour, rect, r, segs);

		SDL_FColor col = COLOR_CAST(color);
		m_mesh.clear();
		u32 centerIndex = m_mesh.addVertex({ rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f }, col);
		for (const auto& p : contour)
			m_mesh.addVertex(p, col);
		const u32 n = cast<u32>(contour.size());
		for (u32 i = 0; i < n; i++)
			m_mesh.addTriangle(centerIndex, 1 + i, 1 + (i + 1) % n);
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::fillCircle(const Vec2& center, f32 radius, const Color& color)
	{
		if (!m_initialized)
			return;
		m_mesh.clear();
		mesh_filled_arc(m_mesh, center, radius, radius, 0.0f, 2.0f * M_PI, circle_segments(radius), COLOR_CAST(color));
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::fillCircle(const Rectangle& rect, const Color& color)
	{
		if (!m_initialized)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radius = std::min(rect.w, rect.h) * 0.5f;
		fillCircle(center, radius, color);
	}

	void BasicRenderer2D::fillEllipse(const Rectangle& rect, const Color& color)
	{
		if (!m_initialized)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radiusX = rect.w * 0.5f;
		f32 radiusY = rect.h * 0.5f;
		m_mesh.clear();
		mesh_filled_arc(m_mesh, center, radiusX, radiusY, 0.0f, 2.0f * M_PI, circle_segments(std::max(radiusX, radiusY)), COLOR_CAST(color));
		submit_flat_mesh(m_mesh);
	}

	void BasicRenderer2D::fillTriangle(const Triangle& tri, const Color& color)
	{
		if (!m_initialized)
			return;
		fillTriangle(tri.vA, tri.vB, tri.vC, color);
	}

	void BasicRenderer2D::fillTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& color)
	{
		if (!m_initialized)
			return;
		SDL_FColor col = COLOR_CAST(color);
		m_mesh.clear();
		m_mesh.addVertex(A, col);
		m_mesh.addVertex(B, col);
		m_mesh.addVertex(C, col);
		m_mesh.addTriangle(0, 1, 2);
		submit_flat_mesh(m_mesh);
	}




	void BasicRenderer2D::outlinedRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness, bool topEdge, bool rightEdge, bool bottomEdge, bool leftEdge)
	{
		if (!m_initialized) return;
		Rectangle offset = { 1, 1, -2, -2 };
		fillRect(rect + offset, fillColor);
		drawRect(rect, outlineColor, outlineThickness, topEdge, rightEdge, bottomEdge, leftEdge);
	}

	void BasicRenderer2D::outlinedRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, i32 outlineThickness, bool topEdge, bool rightEdge, bool bottomEdge, bool leftEdge)
	{
		outlinedRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, fillColor, outlineColor, outlineThickness, topEdge, rightEdge, bottomEdge, leftEdge);
	}

	void BasicRenderer2D::outlinedRoundRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, f32 radius, i32 outlineThickness)
	{
		outlinedRoundRect(rect, fillColor, outlineColor, { radius, radius, radius, radius }, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, f32 radius, i32 outlineThickness)
	{
		outlinedRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, fillColor, outlineColor, { radius, radius, radius, radius }, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const Vec2& center, const Vec2& size, const Color& fillColor, const Color& outlineColor, const Rectangle& radii, i32 outlineThickness)
	{
		outlinedRoundRect({ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y }, fillColor, outlineColor, radii, outlineThickness);
	}

	void BasicRenderer2D::outlinedRoundRect(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, const Rectangle& radii, i32 outlineThickness)
	{
		if (!m_initialized) return;
		Rectangle offset = { 1, 1, -2, -2 };
		fillRoundRect(rect + offset, fillColor, radii);
		drawRoundRect(rect, outlineColor, radii, outlineThickness);
	}

	void BasicRenderer2D::outlinedCircle(const Vec2& center, f32 radius, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		fillCircle(center, radius - 1, fillColor);
		drawCircle(center, radius, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedCircle(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized)
			return;

		Vec2 center {
			rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f
		};

		f32 radius = std::min(rect.w, rect.h) * 0.5f;
		outlinedCircle(center, radius, fillColor, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedEllipse(const Rectangle& rect, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		Rectangle offset = { 1, 1, -2, -2 };
		fillEllipse(rect + offset, fillColor);
		drawEllipse(rect, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedTriangle(const Triangle& tri, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		outlinedTriangle(tri.vA, tri.vB, tri.vC, fillColor, outlineColor, outlineThickness);
	}

	void BasicRenderer2D::outlinedTriangle(const Vec2& A, const Vec2& B, const Vec2& C, const Color& fillColor, const Color& outlineColor, i32 outlineThickness)
	{
		if (!m_initialized) return;
		drawTriangle(A, B, C, outlineColor, outlineThickness);
		// Inset the fill triangle so it doesn't bleed past the outline
		f32 inset = cast<f32>(outlineThickness);
		Vec2 centroid = { (A.x + B.x + C.x) / 3.0f, (A.y + B.y + C.y) / 3.0f };

		auto shrink = [&](const Vec2& v) -> Vec2 {
			Vec2 dir = { centroid.x - v.x, centroid.y - v.y };
			f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len < 0.001f) return v;
			dir.x /= len;
			dir.y /= len;
			return { v.x + dir.x * inset, v.y + dir.y * inset };
		};

		fillTriangle(shrink(A), shrink(B), shrink(C), fillColor);
	}
	// ===================================================== PRIMITIVES =====================================================





	// ===================================================== PRIVATE HELPERS =====================================================
	void BasicRenderer2D::__on_render_event(u32 eventType)
	{
		if (eventType == SDL_EVENT_RENDER_DEVICE_LOST)
		{
			if (!m_deviceLost)
				OX_ERROR("ogfx: the render device of window %u was lost and cannot be recovered.", getWindowID());
			m_deviceLost = true;
			discard_batch();
			return;
		}
		if (eventType == SDL_EVENT_RENDER_TARGETS_RESET)
		{
			// The textures still exist, only render target contents are gone.
			for (auto* target : stdvec<RenderTarget*>(m_targets))
				target->notify_contents_lost();
			return;
		}
		if (eventType == SDL_EVENT_RENDER_DEVICE_RESET)
		{
			// Every texture is gone: queued geometry references them, the glyph atlases are rebuilt
			// from scratch (glyphs get re-rasterized on demand), targets get new (empty) textures
			// and images are reloaded from their files.
			discard_batch();
			unwind_render_targets();
			if (!m_initialized)
				return;
			m_fontGlyphAtlas.rebuild();
			m_lastUsedGlyphAtlasTex = m_fontGlyphAtlas.getAtlas(0);
			for (auto* target : stdvec<RenderTarget*>(m_targets))
			{
				target->release_texture();
				target->ensure_texture();
				target->notify_contents_lost();
			}
			for (auto* image : stdvec<Image*>(m_images))
				image->__reload();
		}
	}

	void BasicRenderer2D::simulateDeviceReset(void)
	{
		__on_render_event(SDL_EVENT_RENDER_DEVICE_RESET);
	}

	f32 BasicRenderer2D::getPixelScale(void) const
	{
		return m_window != nullptr ? m_window->getScaleFactor() : 1.0f;
	}

	bool BasicRenderer2D::beginRenderTarget(RenderTarget& target, bool clear, const Color& clearColor)
	{
		if (!m_initialized)
			return false;
		if (target.m_renderer != this)
		{
			OX_ERROR("BasicRenderer2D::beginRenderTarget(): the target was not created with this renderer.");
			return false;
		}
		for (const auto& binding : m_targetStack)
		{
			if (binding.target == &target)
			{
				OX_ERROR("BasicRenderer2D::beginRenderTarget(): the target is already bound.");
				return false;
			}
		}
		if (!target.ensure_texture())
			return false;

		flushBatch();
		m_targetStack.push_back({ &target, std::move(m_clipStack) });
		m_clipStack.clear();
		bind_sdl_target(&target);
		apply_clip_rect();
		if (clear)
		{
			// Target contents are premultiplied, so is the clear color.
			auto c = clearColor.getNormalizedColor();
			SDL_SetRenderDrawColorFloat(getSDLRenderer(), c.r * c.a, c.g * c.a, c.b * c.a, c.a);
			SDL_RenderClear(getSDLRenderer());
			target.m_contentsLost = false;
		}
		return true;
	}

	void BasicRenderer2D::endRenderTarget(void)
	{
		if (m_targetStack.empty())
		{
			OX_WARN("BasicRenderer2D::endRenderTarget() called without a bound render target.");
			return;
		}
		flushBatch();
		TargetBinding binding = std::move(m_targetStack.back());
		m_targetStack.pop_back();
		m_clipStack = std::move(binding.savedClipStack);
		bind_sdl_target(m_targetStack.empty() ? nullptr : m_targetStack.back().target);
		apply_clip_rect();
	}

	void BasicRenderer2D::drawRenderTarget(const RenderTarget& target, const Vec2& position, const Vec2& size, const Color& tint, const Rectangle& srcRect)
	{
		if (!m_initialized || !target.m_texture)
			return;
		if (target.m_renderer != this)
		{
			OX_ERROR("BasicRenderer2D::drawRenderTarget(): the target was not created with this renderer.");
			return;
		}
		if (!m_targetStack.empty() && m_targetStack.back().target == &target)
		{
			OX_ERROR("BasicRenderer2D::drawRenderTarget(): a render target can't be drawn into itself.");
			return;
		}
		const Vec2 tsize = target.m_size;
		Rectangle src = srcRect;
		if (src.w == 0 && src.h == 0)
			src = { 0, 0, tsize.x, tsize.y };
		f32 dw = (size.x == 0) ? src.w : size.x;
		f32 dh = (size.y == 0) ? src.h : size.y;

		Vec2 verts[4] = {
			{ position.x,      position.y },
			{ position.x + dw, position.y },
			{ position.x + dw, position.y + dh },
			{ position.x,      position.y + dh }
		};
		Vec2 uvs[4] = {
			{ src.x / tsize.x,           src.y / tsize.y },
			{ (src.x + src.w) / tsize.x, src.y / tsize.y },
			{ (src.x + src.w) / tsize.x, (src.y + src.h) / tsize.y },
			{ src.x / tsize.x,           (src.y + src.h) / tsize.y }
		};
		// The contents are premultiplied: the tint's alpha must scale the color channels too.
		SDL_FColor col = COLOR_CAST(tint);
		col.r *= col.a;
		col.g *= col.a;
		col.b *= col.a;
		emit_textured_quad(target.m_texture, verts, uvs, col, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
	}

	bool BasicRenderer2D::areShadersSupported(void) const
	{
		SDL_Renderer* renderer = getSDLRenderer();
		return m_initialized && renderer && SDL_GetGPURendererDevice(renderer) != nullptr;
	}

	void BasicRenderer2D::beginShader(Shader& shader)
	{
		m_shaderStack.push_back(&shader);
		update_active_shader();
	}

	void BasicRenderer2D::endShader(void)
	{
		if (m_shaderStack.empty())
		{
			OX_WARN("BasicRenderer2D::endShader() called without an active shader.");
			return;
		}
		m_shaderStack.pop_back();
		update_active_shader();
	}

	bool BasicRenderer2D::blurRenderTarget(const RenderTarget& source, RenderTarget& destination, f32 radius)
	{
		if (!m_initialized || !source.hasTexture() || &source == &destination)
			return false;
		if (!areShadersSupported() || !ensure_blur_shaders())
		{
			if (!m_warnedNoBlur)
				OX_WARN("ogfx: blur needs shader support (SDL's GPU renderer), which is not available.");
			m_warnedNoBlur = true;
			return false;
		}
		if (!destination.isCreated())
			destination.create(*this, source.getSize());
		if (destination.getRenderer() != this)
		{
			OX_ERROR("BasicRenderer2D::blurRenderTarget(): the destination belongs to another renderer.");
			return false;
		}
		destination.resize(source.getSize());

		BlurTargets& temp = get_blur_targets(source.getSize());
		const IPoint px = source.getPixelSize();
		const f32 reach = std::clamp(radius * source.getPixelScale(), 0.0f, cast<f32>(MaxBlurRadius));
		BlurUniforms uniforms { };
		uniforms.texelSize[0] = 1.0f / cast<f32>(px.x);
		uniforms.texelSize[1] = 1.0f / cast<f32>(px.y);
		uniforms.sigma = std::max(reach / 3.0f, 0.001f);
		uniforms.radius = cast<i32>(std::ceil(reach));

		uniforms.direction[0] = 1.0f;
		uniforms.direction[1] = 0.0f;
		m_blurShaderH->setUniforms(uniforms);
		uniforms.direction[0] = 0.0f;
		uniforms.direction[1] = 1.0f;
		m_blurShaderV->setUniforms(uniforms);

		if (!beginRenderTarget(*temp.pass))
			return false;
		beginShader(*m_blurShaderH);
		drawRenderTarget(source, { 0, 0 });
		endShader();
		endRenderTarget();

		if (!beginRenderTarget(destination))
			return false;
		beginShader(*m_blurShaderV);
		drawRenderTarget(*temp.pass, { 0, 0 });
		endShader();
		endRenderTarget();
		return true;
	}

	void BasicRenderer2D::drawRenderTargetBlurred(const RenderTarget& target, const Vec2& position, f32 radius, const Vec2& size, const Color& tint, const Rectangle& srcRect)
	{
		if (!m_initialized || !target.hasTexture())
			return;
		if (!areShadersSupported())
		{
			if (!m_warnedNoBlur)
				OX_WARN("ogfx: blur needs shader support (SDL's GPU renderer), which is not available; drawing unblurred.");
			m_warnedNoBlur = true;
			drawRenderTarget(target, position, size, tint, srcRect);
			return;
		}
		BlurTargets& temp = get_blur_targets(target.getSize());
		if (!blurRenderTarget(target, *temp.result, radius))
		{
			drawRenderTarget(target, position, size, tint, srcRect);
			return;
		}
		drawRenderTarget(*temp.result, position, size, tint, srcRect);
	}

	void BasicRenderer2D::__release_device_resources(void)
	{
		discard_batch();
		unwind_render_targets();
		m_shaderStack.clear();
		update_active_shader();
		for (auto* shader : stdvec<Shader*>(m_shaders))
			shader->release_renderer(*this);
		for (auto* target : stdvec<RenderTarget*>(m_targets))
			target->release_texture();
		for (auto* image : stdvec<Image*>(m_images))
			image->__release_texture();
		m_fontGlyphAtlas.destroy();
		m_lastUsedGlyphAtlasTex = nullptr;
		m_clipStack.clear();
		closeFont();
		m_initialized = false;
	}

	void BasicRenderer2D::apply_clip_rect(void)
	{
		SDL_Renderer* renderer = getSDLRenderer();
		if (!renderer)
			return;
		if (m_clipStack.empty())
		{
			SDL_SetRenderClipRect(renderer, nullptr);
			return;
		}
		const auto& clip = m_clipStack.back();
		SDL_Rect r;
		r.x = (i32)std::round(clip.x);
		r.y = (i32)std::round(clip.y);
		r.w = (i32)std::round(clip.w);
		r.h = (i32)std::round(clip.h);
		SDL_SetRenderClipRect(renderer, &r);
	}

	void BasicRenderer2D::bind_sdl_target(RenderTarget* target)
	{
		SDL_Renderer* renderer = getSDLRenderer();
		if (!renderer)
			return;
		SDL_SetRenderTarget(renderer, target ? target->m_texture : nullptr);
		// SDL3 keeps viewport/clip/scale per target: a target's view starts at scale 1, so it gets
		// the target's own pixel scale (the window's view keeps the scale the window set).
		if (target)
			SDL_SetRenderScale(renderer, target->m_scale, target->m_scale);
	}

	void BasicRenderer2D::clear_texture(SDL_Texture* texture)
	{
		SDL_Renderer* renderer = getSDLRenderer();
		if (!renderer || !texture)
			return;
		flushBatch();
		SDL_Texture* previous = SDL_GetRenderTarget(renderer);
		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 0.0f);
		SDL_RenderClear(renderer);
		SDL_SetRenderTarget(renderer, previous);
	}

	void BasicRenderer2D::unwind_render_targets(void)
	{
		while (!m_targetStack.empty())
			endRenderTarget();
	}

	void BasicRenderer2D::register_target(RenderTarget& target)
	{
		if (std::find(m_targets.begin(), m_targets.end(), &target) == m_targets.end())
			m_targets.push_back(&target);
	}

	void BasicRenderer2D::unregister_target(RenderTarget& target)
	{
		for (size_t i = 0; i < m_targetStack.size(); i++)
		{
			if (m_targetStack[i].target == &target)
			{
				OX_ERROR("A RenderTarget was destroyed while bound.");
				while (m_targetStack.size() > i)
					endRenderTarget();
				break;
			}
		}
		std::erase(m_targets, &target);
	}

	void BasicRenderer2D::register_image(Image& image)
	{
		if (std::find(m_images.begin(), m_images.end(), &image) == m_images.end())
			m_images.push_back(&image);
	}

	void BasicRenderer2D::unregister_image(Image& image)
	{
		std::erase(m_images, &image);
	}

	void BasicRenderer2D::discard_batch(void)
	{
		m_vertexCount = 0;
		m_indexCount = 0;
		m_batchState = { };
	}

	void BasicRenderer2D::emit(const BatchState& drawState, const SDL_Vertex* verts, u32 vertCount, const i32* inds, u32 indexCount)
	{
		if (!m_initialized || vertCount == 0 || indexCount == 0)
			return;
		BatchState state = drawState;
		// A custom fragment shader samples the draw's texture: untextured draws can't use one.
		state.shader = state.texture ? m_activeShaderState : nullptr;
		if (vertCount > MaxVertices || indexCount > MaxIndices)
		{
			OX_ERROR("Maximum number of vertices exceeded in single draw call.");
			return;
		}
		if (state != m_batchState)
		{
			flushBatch();
			// Textures belong to one SDL_Renderer: using one from another window's renderer is a bug.
			SDL_assert(state.texture == nullptr || SDL_GetRendererFromTexture(state.texture) == getSDLRenderer());
		}
		if (m_vertexCount + cast<i32>(vertCount) > MaxVertices || m_indexCount + cast<i32>(indexCount) > MaxIndices)
			flushBatch();
		m_batchState = state;

		const i32 base = m_vertexCount;
		std::memcpy(&m_vertices[m_vertexCount], verts, sizeof(SDL_Vertex) * vertCount);
		m_vertexCount += cast<i32>(vertCount);
		for (u32 i = 0; i < indexCount; i++)
			m_indices[m_indexCount++] = base + inds[i];
	}

	void BasicRenderer2D::emit_textured_quad(SDL_Texture* texture, const Vec2 verts[4], const Vec2 uvs[4], const SDL_FColor& col, SDL_BlendMode blend)
	{
		const SDL_Vertex quad[4] = {
			{ { verts[0].x, verts[0].y }, col, { uvs[0].x, uvs[0].y } },
			{ { verts[1].x, verts[1].y }, col, { uvs[1].x, uvs[1].y } },
			{ { verts[2].x, verts[2].y }, col, { uvs[2].x, uvs[2].y } },
			{ { verts[3].x, verts[3].y }, col, { uvs[3].x, uvs[3].y } }
		};
		const i32 inds[6] = QUAD_INDICES_ARR;
		emit({ texture, blend }, quad, 4, inds, 6);
	}

	void BasicRenderer2D::submit_flat_mesh(const FlatMesh& mesh, bool antialias)
	{
		if (!m_initialized)
			return;
		m_meshProcessor->process(mesh, m_fontGlyphAtlas.getNullTextureSlotUVs(), antialias, m_meshVerts, m_meshInds);
		emit(flat_batch_state(), m_meshVerts.data(), cast<u32>(m_meshVerts.size()), m_meshInds.data(), cast<u32>(m_meshInds.size()));
	}

	BasicRenderer2D::BatchState BasicRenderer2D::flat_batch_state(void) const
	{
		// Every atlas has the white texel block at the same spot, so flat shapes sample whichever
		// atlas text was last drawn with - that way text and shapes share batches.
		return { m_lastUsedGlyphAtlasTex, SDL_BLENDMODE_BLEND };
	}

	bool BasicRenderer2D::mesh_line(FlatMesh& mesh, const Vec2& p1, const Vec2& p2, f32 thickness, bool rounded, const SDL_FColor& color)
	{
		Vec2 delta = p2 - p1;
		if (delta.mag() < 1e-4f || thickness <= 0.0f)
			return false;
		Vec2 dir = delta.normalize();
		Vec2 perp = { -dir.y, dir.x }; // 90° rotation
		f32 half = thickness * 0.5f;
		Vec2 off = perp * half;

		u32 a = mesh.addVertex(p1 - off, color);
		u32 b = mesh.addVertex(p1 + off, color);
		u32 c = mesh.addVertex(p2 + off, color);
		u32 d = mesh.addVertex(p2 - off, color);
		mesh.addTriangle(a, b, c);
		mesh.addTriangle(c, d, a);

		if (!rounded)
			return true;

		// Half-circle caps: their end points coincide with the quad's corners, so after welding the
		// caps and the body are one mesh (no fringe along the joins).
		i32 segments = std::max(16, cast<i32>(thickness * 1.5f));
		f32 angle1 = std::atan2(-dir.y, -dir.x);
		f32 angle2 = std::atan2(dir.y, dir.x);
		mesh_filled_arc(mesh, p1, half, half, angle1 - M_PI * 0.5f, angle1 + M_PI * 0.5f, segments, color);
		mesh_filled_arc(mesh, p2, half, half, angle2 - M_PI * 0.5f, angle2 + M_PI * 0.5f, segments, color);
		return true;
	}

	void BasicRenderer2D::mesh_filled_arc(FlatMesh& mesh, const Vec2& center, f32 radiusX, f32 radiusY, f32 startAngle, f32 endAngle, i32 segments, const SDL_FColor& color)
	{
		if (segments <= 0)
			return;
		u32 centerIndex = mesh.addVertex(center, color);
		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = cast<f32>(i) / cast<f32>(segments);
			f32 angle = startAngle + t * (endAngle - startAngle);
			mesh.addVertex({ center.x + std::cos(angle) * radiusX, center.y + std::sin(angle) * radiusY }, color);
		}
		for (i32 i = 0; i < segments; i++)
			mesh.addTriangle(centerIndex, centerIndex + 1 + i, centerIndex + 2 + i);
	}

	void BasicRenderer2D::mesh_arc_stroke(FlatMesh& mesh, const Vec2& center, f32 radiusX, f32 radiusY, f32 thickness, f32 startAngle, f32 endAngle, i32 segments, const SDL_FColor& color)
	{
		if (segments <= 0)
			return;
		f32 half = thickness * 0.5f;
		f32 outerRX = radiusX + half;
		f32 outerRY = radiusY + half;
		f32 innerRX = std::max(0.0f, radiusX - half);
		f32 innerRY = std::max(0.0f, radiusY - half);

		u32 base = cast<u32>(mesh.positions.size());
		for (i32 i = 0; i <= segments; i++)
		{
			f32 t = cast<f32>(i) / cast<f32>(segments);
			f32 angle = startAngle + t * (endAngle - startAngle);
			f32 cs = std::cos(angle);
			f32 sn = std::sin(angle);
			mesh.addVertex({ center.x + cs * outerRX, center.y + sn * outerRY }, color);
			mesh.addVertex({ center.x + cs * innerRX, center.y + sn * innerRY }, color);
		}
		for (i32 i = 0; i < segments; i++)
		{
			u32 o0 = base + i * 2;
			u32 i0 = o0 + 1;
			u32 o1 = o0 + 2;
			u32 i1 = o0 + 3;
			mesh.addTriangle(o0, o1, i0);
			mesh.addTriangle(o1, i1, i0);
		}
	}

	void BasicRenderer2D::register_shader(Shader& shader)
	{
		if (std::find(m_shaders.begin(), m_shaders.end(), &shader) == m_shaders.end())
			m_shaders.push_back(&shader);
	}

	void BasicRenderer2D::unregister_shader(Shader& shader, SDL_GPURenderState* state)
	{
		flush_if_using(state);
		std::erase(m_shaders, &shader);
		std::erase(m_shaderStack, &shader);
		update_active_shader();
	}

	void BasicRenderer2D::flush_if_using(SDL_GPURenderState* state)
	{
		if (state && m_vertexCount > 0 && m_batchState.shader == state)
			flushBatch();
	}

	void BasicRenderer2D::update_active_shader(void)
	{
		m_activeShaderState = m_shaderStack.empty() ? nullptr : m_shaderStack.back()->get_render_state(*this);
	}

	bool BasicRenderer2D::ensure_blur_shaders(void)
	{
		if (m_blurShaderH && m_blurShaderV)
			return m_blurShaderH->isLoaded() && m_blurShaderV->isLoaded();
		// One shader per direction: each keeps its own uniforms, so the two passes don't force
		// a uniform update (and a GPU flush) between them. They share the compiled GPU shader.
		m_blurShaderH = std::make_unique<Shader>();
		m_blurShaderV = std::make_unique<Shader>();
		const ShaderBytecode bytecode = OGFX_EMBEDDED_SHADER_BYTECODE(blur_frag);
		const ShaderLayout layout = OGFX_EMBEDDED_SHADER_LAYOUT(blur_frag);
		return m_blurShaderH->loadFromMemory(bytecode, layout) && m_blurShaderV->loadFromMemory(bytecode, layout);
	}

	BasicRenderer2D::BlurTargets& BasicRenderer2D::get_blur_targets(const Vec2& size)
	{
		BlurTargets* lru = nullptr;
		for (auto& entry : m_blurCache)
		{
			if (entry->size.x == size.x && entry->size.y == size.y)
			{
				entry->lastUsedFrame = m_frameIndex;
				return *entry;
			}
			if (!lru || entry->lastUsedFrame < lru->lastUsedFrame)
				lru = entry.get();
		}
		// Cache full: reuse the least recently used pair (e.g. while a window is being resized).
		if (m_blurCache.size() >= MaxBlurCacheEntries && lru)
		{
			lru->size = size;
			lru->pass->resize(size);
			lru->result->resize(size);
			lru->lastUsedFrame = m_frameIndex;
			return *lru;
		}
		auto entry = std::make_unique<BlurTargets>();
		entry->size = size;
		entry->pass = std::make_unique<RenderTarget>(*this, size);
		entry->result = std::make_unique<RenderTarget>(*this, size);
		entry->lastUsedFrame = m_frameIndex;
		m_blurCache.push_back(std::move(entry));
		return *m_blurCache.back();
	}

	i32 BasicRenderer2D::get_kerning(TTF_Font* font, i32 fontSize, u32 left, u32 right)
	{
		const KernKey key { cast<u64>(reinterpret_cast<uintptr_t>(font)), cast<u32>(fontSize), left, right };
		auto it = m_kernCache.find(key);
		if (it != m_kernCache.end())
			return it->second;
		i32 kern = 0;
		const f32 oldSize = TTF_GetFontSize(font);
		if (oldSize != cast<f32>(fontSize))
			TTF_SetFontSize(font, cast<f32>(fontSize));
		TTF_GetGlyphKerning(font, left, right, &kern);
		if (oldSize != cast<f32>(fontSize))
			TTF_SetFontSize(font, oldSize);
		if (m_kernCache.size() >= MaxKernCacheSize)
			m_kernCache.clear();
		m_kernCache[key] = kern;
		return kern;
	}

	void BasicRenderer2D::print_ttf_error(const String& funcName)
	{
		m_out.fg(ostd::ConsoleColors::Red).p(funcName).p("(...) failed: ").p(String(SDL_GetError())).reset().nl();
	}
	// ===================================================== PRIVATE HELPERS =====================================================
}
