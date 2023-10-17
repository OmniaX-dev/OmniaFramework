#include "BlendRenderer.hpp"
#include <ostd/Signals.hpp>
#include <ogfx/RenderCore.hpp>

namespace ogfx
{
	BlendRenderer& BlendRenderer::create(IPoint screenSize, OrthoCamera& camera)
	{
		m_screenSize = screenSize;
		m_destination.create(screenSize.x, screenSize.y);
		m_shader = ResourceManager::getDefaultBlendShader();
		m_blendModes.resize(BlendRenderer::MaxLayers, tBlendModes::Transparent);;
		connectSignal(tBuiltinSignals::WindowResized);
		m_camera = &camera;
		setTypeName("ox::BlendRenderer");
		validate();
		return *this;
	}

	LayerID BlendRenderer::addLayer(RenderTarget& target, uint8_t blendMode)
	{//TODO: Errors
		if (isInvalid()) return BlendRenderer::InvalidLayer;
		if (m_layers.size() >= BlendRenderer::MaxLayers) return BlendRenderer::InvalidLayer;
		LayerID id = BlendRenderer::s_nextID++;
		m_layers.push_back({ target, blendMode });
		return id;
	}

	void BlendRenderer::process(RenderCore& gfx)
	{
		if (isInvalid()) return;
		uint32_t rt = gfx.getCurrentRenderTarget();
		ResourceID shad = gfx.getCurrentShaderID();
		gfx.setRenderTarget(m_destination);
		gfx.bindShader(m_shader);
		gfx.clear({ 0, 0, 0, 0 });
		gfx.updateUniform_mat4f("u_viewProjMatrix", m_camera->getProjectionMatrix());
		uint32_t index = 0;
		for (auto& layer : m_layers)
		{
			m_blendModes[index] = layer.blendMode;
			gfx.drawRenderTarget(*layer.target, { 0.0f, 0.0f }, { (float)m_screenSize.x, (float)m_screenSize.y });
			// gfx.drawRenderTarget(*layer.target);//, { 0.0f, 0.0f }, { (float)m_screenSize.x, (float)m_screenSize.y });
			index++;
		}
		if (index < BlendRenderer::MaxLayers)
		{
			for ( ; index < BlendRenderer::MaxLayers; index++)
				m_blendModes[index] = tBlendModes::Transparent;
		}
		gfx.updateUniform_arri("u_blendModes", m_blendModes);
		gfx.updateUniform_i("u_layerCount", m_layers.size());
		gfx.bindShader(shad);
		gfx.setRenderTarget(rt);
	}

	void BlendRenderer::renderTo(RenderCore& gfx, const RenderTarget& target, ResourceID shader)
	{
		if (isInvalid()) return;
		uint32_t rt = gfx.getCurrentRenderTarget();
		ResourceID shad = gfx.getCurrentShaderID();
		gfx.setRenderTarget(target);
		gfx.bindShader(shader);
		gfx.updateUniform_mat4f("u_viewProjMatrix", m_camera->getProjectionMatrix());
		gfx.drawRenderTarget(m_destination);
		gfx.bindShader(shad);
		gfx.setRenderTarget(rt);
	}

	bool BlendRenderer::overrideDefaultBlendShader(ResourceID shader)
	{
		if (ResourceManager::getShader(shader).isInvalid()) return false;
		m_shader = shader;
		return true;
	}

	void BlendRenderer::renderToScreen(RenderCore& gfx, ResourceID shader)
	{
		renderTo(gfx, Renderer2D::getDefaultRenderTarget(), shader);
	}

	void BlendRenderer::setDefaultBlendShader(void)
	{
		m_shader = ResourceManager::getDefaultBlendShader();
	}

	BlendRenderer::tLayer& BlendRenderer::getLayer(LayerID layer)
	{
		if (layer >= m_layers.size()) return BlendRenderer::s_invalidLayer;
		return m_layers[layer];
	}

	void BlendRenderer::handleSignal(tSignal& signal)
	{
		if (signal.ID == tBuiltinSignals::WindowResized)
		{
			WindowSizeObj& newSize = static_cast<WindowSizeObj&>(signal.userData);
			m_screenSize = { newSize.width, newSize.height };
		}
	}
}