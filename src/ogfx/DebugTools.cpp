#include "DebugTools.hpp"

#include <ostd/Signals.hpp>
#include <ogfx/RenderCore.hpp>
#include <ogfx/GraphicsApplication.hpp>

namespace ogfx
{
	void DebugInfoWidget::init(ogfx::oxgui::Widget& parent) 
	{
		if (parent.isInvalid())
		{
			invalidate();
		}
		setPosition({ 0, 0 });
		setSize(parent.getContentSize());

		connectSignals();
		connectSignal(DebugTools::OnProfileDataRecieved);
		setTypeName("ogfx::DebugInfoWidget");
		validate();
		enable();
		parent.addWidget(*this);
	}

	void DebugInfoWidget::renderContent(ogfx::RenderCore& gfx)
	{
		if (m_dataList == nullptr) return;

		Renderer2D::tTextInfo textInfo;
		textInfo.characterHeight = 24;
		textInfo.characterSpacing = 6;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		ogfx::Renderer2D::Text::characterHeight = textInfo.characterHeight;
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;

		auto stateGradientPixels = ogfx::ResourceManager::getTexture(ogfx::ResourceManager::getStateGradientTexture()).getPixelData();

		ostd::Vec2 pos = getPosition() + ostd::Vec2 { 10, 20 };

		bool line = true;

		float max_avg = 0;
		for (auto& data : m_dataList->data)
		{
			if (ostd::StringEditor(data.second.name).startsWith("Total Frame Time"))
				continue;
			float avg = data.second.getAvgTime();
			if (avg > max_avg)
				max_avg = avg;
		}

		for (auto& data : m_dataList->data)
		{
			if (!line)
				gfx.drawQuad({ getx() + 1, pos.y }, { getw() - 2, 20 }, { 30, 30, 30, 255 }, false);
			line = !line;

			float avg_val = data.second.getAvgTime();
			uint32_t mapped = std::round(ostd::Utils::map_value(avg_val, 0.0f, max_avg, 1.0f, 198.0f)) * 4;

			textInfo.characterSpacing = 6;
			textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
			ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
			ogfx::Renderer2D::Text::font = textInfo.font;
			textInfo.color = { stateGradientPixels[mapped], stateGradientPixels[mapped + 1], stateGradientPixels[mapped + 2] };
			if (ostd::StringEditor(data.second.name).startsWith("Total Frame Time"))
				textInfo.color = { 200, 200, 200 };
			ostd::StringEditor str(data.second.name);
			str.add(":  ");
			gfx.drawText(str.str(), pos, textInfo);


			str.clr().addf(avg_val).add(" ms");
			textInfo.characterSpacing = -10;
			textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
			ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
			ogfx::Renderer2D::Text::font = textInfo.font;
			float w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
			gfx.drawText(str.str(), { getx() + getw() - w + 35, pos.y }, textInfo);

			pos += ostd::Vec2 { 0, 20 };
		}

		textInfo.characterHeight = 24;
		textInfo.characterSpacing = 6;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.color = { 30, 150, 200 };
		ogfx::Renderer2D::Text::characterHeight = textInfo.characterHeight;
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;

		ostd::StringEditor str;

		gfx.drawQuad({ getx() + 1, pos.y }, { getw() - 2, 20 }, { 30, 30, 70, 200 }, false);
		gfx.drawText("FPS: ", pos, textInfo);
		str.addi(DebugTools::m_fps);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		float w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		// std::cout << str << "\n";
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };

		gfx.drawText("UPS: ", pos, textInfo);
		str.addi(DebugTools::m_ups);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };

		gfx.drawQuad({ getx() + 1, pos.y }, { getw() - 2, 20 }, { 30, 30, 70, 200 }, false);
		gfx.drawText("Uniform Updates: ", pos, textInfo);
		str.addi(DebugTools::m_uniformUpdates);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };

		gfx.drawText("Render Queue: ", pos, textInfo);
		str.addi(DebugTools::m_renderCmdCount);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };

		gfx.drawQuad({ getx() + 1, pos.y }, { getw() - 2, 20 }, { 30, 30, 70, 200 }, false);
		gfx.drawText("Draw Calls: ", pos, textInfo);
		str.addi(ogfx::Renderer2D::getRenderStats().drawCalls);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };

		gfx.drawText("Quad Count: ", pos, textInfo);
		str.addi(ogfx::Renderer2D::getRenderStats().quadCount);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };

		gfx.drawQuad({ getx() + 1, pos.y }, { getw() - 2, 20 }, { 30, 30, 70, 200 }, false);
		gfx.drawText("RenderBuffer Binds: ", pos, textInfo);
		str.addi(ogfx::Renderer2D::getRenderStats().renderBufferBinds);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };

		gfx.drawText("Shader Binds: ", pos, textInfo);
		str.addi(ogfx::Renderer2D::getRenderStats().shaderBinds);
		textInfo.characterSpacing = -5;
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFontMono();
		textInfo.color = { 190, 0, 190 };
		ogfx::Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
		ogfx::Renderer2D::Text::font = textInfo.font;
		w = ogfx::Renderer2D::Text::getStringBounds(str.str()).x;
		gfx.drawText(str.str(), { getx() + getw() - 10 - w, pos.y }, textInfo);
		textInfo.color = { 30, 150, 200 };
		textInfo.font = ogfx::ResourceManager::getDefaultBitmapFont();
		textInfo.characterSpacing = 6;
		str.clr();
		pos += ostd::Vec2 { 0, 20 };
	}

	void DebugInfoWidget::onSignal(ostd::tSignal& signal)
	{
		if (signal.ID == DebugTools::OnProfileDataRecieved && m_dataList == nullptr)
		{
			m_dataList = (DebugTimesList*)(&signal.userData);
		}
	}



	void DebugTools::showDebugWindow(GraphicsApplication2D* parent_app, bool show)
	{
		if (parent_app == nullptr) return;
		if (!m_debugWindowEnabled) return;
		parent_app->m_debugToolsWindow.enable(show);
		m_debugWindowTabPanel.selectTab((show ? 0 : 1));
	}
	
}