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

#pragma once

#include <ostd/data/Object.hpp>
#include <ostd/data/Color.hpp>
#include <ostd/math/Geometry.hpp>
#include <functional>

struct SDL_Texture;

namespace ogfx
{
	class BasicRenderer2D;

	// ============================================================================
	// RenderTarget — an offscreen surface a BasicRenderer2D can draw into, and
	// that can then be drawn like an image.
	//
	//   ogfx::RenderTarget rt;
	//   rt.create(gfx, { 400, 300 });        // logical size (same units as all draw calls)
	//   gfx.beginRenderTarget(rt);           // clears to transparent by default
	//       gfx.fillRoundRect(...);
	//   gfx.endRenderTarget();
	//   gfx.drawRenderTarget(rt, position);
	//
	// Size and HiDPI: the size is in logical units, like everything else the
	// renderer draws. The texture is allocated at size * the window's scale factor
	// (getPixelSize()), and drawing into the target uses that same scale, so a
	// target drawn 1:1 is pixel-exact. When the scale factor changes, the texture
	// is re-created the next time the target is bound.
	//
	// Contents: everything rendered into a target is stored with premultiplied
	// alpha (that is what straight-alpha blending onto transparent pixels
	// produces), and drawRenderTarget() composites it accordingly, so shapes look
	// the same drawn directly or through a target.
	//
	// Loss: on SDL_EVENT_RENDER_TARGETS_RESET / SDL_EVENT_RENDER_DEVICE_RESET the
	// contents are gone (the texture is re-created as needed): contentsLost()
	// becomes true and the optional callback runs, so the owner knows to redraw.
	// contentsLost() goes back to false the next time the target is cleared by
	// beginRenderTarget().
	//
	// Lifetime: a target can be created before its renderer is initialized (the
	// texture is created on first use), and can outlive the renderer (it then
	// just becomes empty).
	// ============================================================================
	class RenderTarget : public ostd::Object
	{
		public: using ContentsLostCallback = std::function<void(RenderTarget& target)>;
		public:
			inline RenderTarget(void) { setTypeName("ogfx::RenderTarget"); }
			inline RenderTarget(BasicRenderer2D& renderer, const Vec2& size) { setTypeName("ogfx::RenderTarget"); create(renderer, size); }
			~RenderTarget(void);
			RenderTarget(const RenderTarget&) = delete;
			RenderTarget& operator=(const RenderTarget&) = delete;

			bool create(BasicRenderer2D& renderer, const Vec2& size);
			void destroy(void);
			// Changes the logical size (contents are lost when the pixel size changes).
			bool resize(const Vec2& size);

			inline Vec2 getSize(void) const { return m_size; }
			inline IPoint getPixelSize(void) const { return m_pixelSize; }
			inline f32 getPixelScale(void) const { return m_scale; }
			inline bool isCreated(void) const { return m_renderer != nullptr; }
			inline bool hasTexture(void) const { return m_texture != nullptr; }
			inline BasicRenderer2D* getRenderer(void) const { return m_renderer; }

			// Filtering used when the target is drawn scaled (and when shaders sample it):
			// smooth (bilinear, the default) or nearest-neighbour.
			void setSmoothFiltering(bool smooth);
			inline bool isSmoothFiltering(void) const { return m_smoothFiltering; }

			inline bool contentsLost(void) const { return m_contentsLost; }
			inline void setContentsLostCallback(ContentsLostCallback callback) { m_contentsLostCallback = std::move(callback); }

		private:
			bool ensure_texture(void);
			void release_texture(void);
			void notify_contents_lost(void);
			void detach_from_renderer(void);

		private:
			BasicRenderer2D* m_renderer { nullptr };
			SDL_Texture* m_texture { nullptr };
			Vec2 m_size { 0, 0 };
			IPoint m_pixelSize { 0, 0 };
			f32 m_scale { 1.0f };
			bool m_contentsLost { false };
			bool m_smoothFiltering { true };
			ContentsLostCallback m_contentsLostCallback { nullptr };

			friend class BasicRenderer2D;
	};

	// Binds a render target for the lifetime of the object.
	class ScopedRenderTarget
	{
		public:
			ScopedRenderTarget(BasicRenderer2D& renderer, RenderTarget& target, bool clear = true, const Color& clearColor = Colors::Transparent);
			~ScopedRenderTarget(void);
			ScopedRenderTarget(const ScopedRenderTarget&) = delete;
			ScopedRenderTarget& operator=(const ScopedRenderTarget&) = delete;
			// False when the target could not be bound (drawing then goes to the previous target).
			inline bool isActive(void) const { return m_active; }

		private:
			BasicRenderer2D& m_renderer;
			bool m_active { false };
	};
}
