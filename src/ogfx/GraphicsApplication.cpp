#include "GraphicsApplication.hpp"
#include "Errors.hpp"
#include "Renderer2D.hpp"
#include "Signals.hpp"
#include "RenderCommands.hpp"
#include "RenderCore.hpp"
#include "Utils.hpp"
#include "RTData.hpp"
#include "Widgets.hpp"

namespace ogfx
{
	using namespace ostd;

	GraphicsApplication2D& GraphicsApplication2D::create(int32_t windowWidth, int32_t windowHeight, const ostd::String& windowTitle, tContextSettings contextSettings)
	{
		/** Window Code **/
		m_window.create({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) }, windowTitle, contextSettings);
		if (!m_window.isValid())
		{
			ErrorHandler::pushError(GraphicsApplication2D::ERR_WINDOW_CREATE_FAIL);
			String err_str = ErrorHandler::getLastErrorString();
			OX_FATAL("%s", err_str.c_str());
			return *this;
		}
		
		m_window.printOpenGLInfo();
		// m_window.setWindowIcon(windowIconPath);

		/** Subsystem initialization **/
		Renderer2D::init();
		SignalHandler::init();
		ResourceManager::init();
		Renderer2D::Text::init();
		// Input::init(m_window.getGLFWWindowPtr());
		RenderCommandPool::init();
		RenderCore::initRenderQUeue();
		oxgui::WidgetManager::init();
		/******************************/

		m_lastFrameTime = Timer::getEpoch() / 1000.0;

		enableVSync(false);
	
		connectSignal(tBuiltinSignals::OnGuiEvent);


		if (ogfx::DebugTools::m_debugWindowEnabled)
		{
			DebugTools::m_timesList.updateEntry(DebugTools::FramerateCalculations);
			DebugTools::m_timesList.get(DebugTools::FramerateCalculations).name = "Framerate Calculations";

			DebugTools::m_timesList.updateEntry(DebugTools::OnSecondsUpdate);
			DebugTools::m_timesList.get(DebugTools::OnSecondsUpdate).name = "Seconds Update";

			DebugTools::m_timesList.updateEntry(DebugTools::EventHandling);
			DebugTools::m_timesList.get(DebugTools::EventHandling).name = "Event Handling";

			DebugTools::m_timesList.updateEntry(DebugTools::OnUpdate);
			DebugTools::m_timesList.get(DebugTools::OnUpdate).name = "Update";

			DebugTools::m_timesList.updateEntry(DebugTools::OnRender);
			DebugTools::m_timesList.get(DebugTools::OnRender).name = "Render";

			DebugTools::m_timesList.updateEntry(DebugTools::OnDisplay);
			DebugTools::m_timesList.get(DebugTools::OnDisplay).name = "Render Queue Execution";

			DebugTools::m_timesList.updateEntry(DebugTools::OnFrameFinalize);
			DebugTools::m_timesList.get(DebugTools::OnFrameFinalize).name = "Frame Finalize";

			DebugTools::m_timesList.updateEntry(DebugTools::OnSignalsRefresh);
			DebugTools::m_timesList.get(DebugTools::OnSignalsRefresh).name = "Signals Refresh";

			DebugTools::m_timesList.updateEntry(DebugTools::OnWindowDisplay);
			DebugTools::m_timesList.get(DebugTools::OnWindowDisplay).name = "Flip Buffers";

			DebugTools::m_timesList.updateEntry(DebugTools::TotalFrameTime);
			DebugTools::m_timesList.get(DebugTools::TotalFrameTime).name = "Total Frame Time";

			m_debugToolsWindow.create({ 5, 5 }, { 600, 410 });
			m_debugToolsWindow.enable();
			m_debugToolsWindow.setTitle("Debug Tools");
			// m_debugToolsWindow.setBounds(5, 5, 500, 385);
			oxgui::WidgetTheme theme = oxgui::WidgetManager::getGlobalTheme();
			oxgui::WidgetManager::addWidget(m_debugToolsWindow);
			theme.subWindow.titleBarColor_focused = { 85, 0, 85 };
			theme.subWindow.backgroundColor = { 0, 0, 0, 235 };
			theme.subWindow.borderColor_focused = { 85, 0, 85 };
			m_debugToolsWindow.setTheme(theme);
			m_debugToolsWindow.enableGlobalThemeOverride(true);
			theme.tabPanel.backgroundColor = { 10, 10, 10 };
			theme.tabPanel.tabBackground_focused = { 10, 10, 10 };
			// theme.tabPanel.tabBarBackground = { 85, 0, 85 };
			DebugTools::m_debugWindowTabPanel.setTheme(theme);
			DebugTools::m_debugWindowTabPanel.enableGlobalThemeOverride(true);

			DebugTools::m_debugWindowTabPanel.create(m_debugToolsWindow);
			DebugTools::m_debugWindowTabPanel.addTab(DebugTools::m_debugWindowInfoTab);
			DebugTools::m_debugWindowTabPanel.addTab(DebugTools::m_debugWindowWatcherTab);
			DebugTools::m_debugWindowTabPanel.addTab(DebugTools::m_debugWindowTab3);
			DebugTools::m_debugWindowInfoTab.setText("Debug Info");
			DebugTools::m_debugWindowWatcherTab.setText("Watcher");
			DebugTools::m_debugWindowTab3.setText("Tab3");
			DebugTools::m_debugWindowTabPanel.selectTab(0);

			DebugTools::m_debugInfoWidget.init(DebugTools::m_debugWindowInfoTab);
		}

		setTypeName("ogfx::GraphicsApplication2D");
		validate();
		onSetup();
		m_window.initialize();
		return *this;
	}

	void GraphicsApplication2D::destroy(void)
	{
		RenderCommandPool::destroy();
		Renderer2D::shutdown();
	}

	void GraphicsApplication2D::nextFrame(void)
	{
		if (!isRunning()) return;

		const bool DEBUG = ogfx::DebugTools::m_debugWindowEnabled;

		if (DEBUG)
		{
			DebugTools::m_totalFrameTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		double currTime = Timer::getEpoch() / 1000.0;
		m_rtfps++;
		RTData::DeltaTime = (float)(currTime - m_lastFrameTime);
		m_lastFrameTime = currTime;
		double timeDiff = currTime - m_prevTime;
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::FramerateCalculations).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::FramerateCalculations).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		if (timeDiff >= 1.0f)
		{
			onSecondsUpdate();
			// SoundHandler::update();
			m_prevTime = currTime;
			if (DEBUG)
			{
				DebugTools::m_fps = m_rtfps;
				DebugTools::m_ups = m_rtups;
				for (auto& data : DebugTools::m_timesList.data)
				{
					data.second.computeAvgTime();
					data.second.sampleCount = 1;
					data.second.totalTime = data.second.getAvgTime();
				}
			}
			m_rtfps = 0;
			m_rtups = 0;
		}
		// LightingManager::update();
		onFrameStart();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnSecondsUpdate).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnSecondsUpdate).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		m_window.handleEvents();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::EventHandling).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::EventHandling).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		while (m_upsAccumulator > m_upsStep)
		{
			onUpdate();
        	m_upsAccumulator -= m_upsStep;
			m_rtups++;
			if (DEBUG)
			{
				SignalHandler::emitSignal(DebugTools::OnProfileDataRecieved, tSignalPriority::RealTime, DebugTools::m_timesList);
				ogfx::DebugTools::m_drawBasicFrameInfo = ogfx::DebugTools::m_debugWindowTabPanel.getSelectedTab() != 0;
			}
		}
    	m_upsAccumulator += m_upsClock.restart();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnUpdate).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnUpdate).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		onRender();
		onGuiRender();
		Renderer2D::resetStats();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnRender).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnRender).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		RenderCore::display();
		if (DEBUG)
		{
			DebugTools::m_renderCmdCount = RenderCore::getRenderCmdCount();
		}
		RenderCore::resetRenderCmdCount();
		Renderer2D::setDefaultRenderTarget();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnDisplay).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnDisplay).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		SignalHandler::refresh();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnSignalsRefresh).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnSignalsRefresh).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		m_window.renderFrame();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnWindowDisplay).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnWindowDisplay).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		onFrameEnd();
		RenderCommandPool::newFrame();
		if (DEBUG)
		{
			DebugTools::m_uniformUpdates = Shader::getUniformUpdateCount();
		}
		Shader::resetUniformUpdateCount();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnFrameFinalize).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnFrameFinalize).sampleCount++;
		}
		if (!isRunning())
		{
			destroy();
			OX_DEBUG("Destroying SFML Window");
		}
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::TotalFrameTime).totalTime += DebugTools::m_totalFrameTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::TotalFrameTime).sampleCount++;
			for (auto& data : DebugTools::m_timesList.data)
			{
				data.second.computeAvgTime();
			}
		}
	}

	void GraphicsApplication2D::handleSignal(tSignal& signal)
	{
		if (signal.ID == tBuiltinSignals::OnGuiEvent)
		{
			Event& evt = (Event&)signal.userData;
			sf::Event& event = evt.sf();
			if (event.type == sf::Event::Closed)
			{
				m_window.close();
				onClose();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					m_window.close();
					onClose();
				}
				onKeyPressed(evt);
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				// if (event.key.code == sf::Keyboard::F9)
				// 	m_showDebugInfo = !m_showDebugInfo;
				onKeyReleased(evt);
			}
			else if (event.type == sf::Event::Resized)
			{
				glViewport(0, 0, event.size.width, event.size.height);
				m_window.initialize();
			}
			else if (event.type == sf::Event::MouseMoved)
			{
				onMousePressed(evt);
			}
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				onMouseReleased(evt);
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				onMouseMoved(evt);
			}
		}

		onSignal(signal);
	}

	void GraphicsApplication2D::drawDebugInfo(RenderCore& gfx, Color col)
	{
		if (!ogfx::DebugTools::m_debugWindowEnabled || !ogfx::DebugTools::m_drawBasicFrameInfo) return;
		gfx.drawText(StringEditor("FPS: ").addi(ogfx::DebugTools::m_fps).str(), { 5, 5 }, 30.0f, col);
		gfx.drawText(StringEditor("FrameTime: ").addf(DebugTools::m_timesList.get(DebugTools::TotalFrameTime).getAvgTime()).str(), { 5, 35 }, 30.0f, col);
	}

	// void GraphicsApplication2D::drawDebugInfo(RenderCore& gfx, Color col)
	// {
	// 	if (!m_showDebugInfo) return;
	// 	float sp = gfx.getTextSpacing();
	// 	ResourceID font = gfx.getFont();
	// 	Renderer2D::Text::font = ResourceManager::getDefaultBitmapFontMono();
	// 	gfx.setTextSpacing(-15.0f);
	// 	gfx.setFont(ResourceManager::getDefaultBitmapFontMono());
	// 	float fontSize = 24;
	// 	float fontSpacing = -15;

	// 	float strw = 0;
	// 	float maxw = 0;
	// 	std::vector<String> debugStrings;
	// 	debugStrings.reserve(100);
	// 	StringEditor str;
		
	// 	str = "DrawCalls: ";
	// 	str.addi(Renderer2D::getRenderStats().drawCalls);
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;
		
	// 	str = "Quads: ";
	// 	str.addi(Renderer2D::getRenderStats().quadCount);
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;
		
	// 	str = "FPS: ";
	// 	str.addi(getFps());
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;
		
	// 	str = "UPS: ";
	// 	str.addi(m_ups);
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;
		
	// 	str = "RenderTarget Binds: ";
	// 	str.addi(Renderer2D::getRenderStats().renderBufferBinds);
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;
		
	// 	str = "Shader Binds: ";
	// 	str.addi(Renderer2D::getRenderStats().shaderBinds);
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;
		
	// 	str = "Uniform Updates: ";
	// 	str.addi(m_uniformUpdates);
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;
		
	// 	str = "Render Queue: ";
	// 	str.addi(m_renderCmdCount);
	// 	strw = Renderer2D::Text::getStringBounds(str.str(), { 0, 0 }, fontSize, fontSpacing).x;
	// 	debugStrings.push_back(str.str());
	// 	if (strw > maxw) maxw = strw;

	// 	Renderer2D::Text::font = font;

	// 	gfx.drawQuad({ 0, 0 }, { (maxw * 2) + 40, (debugStrings.size() * fontSize * 2) + 15 }, { 0, 0, 0, 223 });

	// 	float offsety = 20;
	// 	int32_t i = 0; 
	// 	for (auto& str : debugStrings)
	// 		gfx.drawText(str, { 5.0f, 10.0f + (i++ * offsety) }, fontSize, col);

	// 	gfx.setTextSpacing(sp);
	// 	gfx.setFont(font);
	// }








	GraphicsApplication& GraphicsApplication::create(int32_t windowWidth, int32_t windowHeight, const ostd::String& windowTitle, tContextSettings contextSettings)
	{
		/** Window Code **/
		m_window.create({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) }, windowTitle, contextSettings);
		if (!m_window.isValid())
		{
			ErrorHandler::pushError(GraphicsApplication::ERR_WINDOW_CREATE_FAIL);
			String err_str = ErrorHandler::getLastErrorString();
			OX_FATAL("%s", err_str.c_str());
			return *this;
		}
		
		m_window.printOpenGLInfo();
		// m_window.setWindowIcon(windowIconPath);

		/** Subsystem initialization **/
		Renderer2D::init();
		SignalHandler::init();
		ResourceManager::init();
		Renderer2D::Text::init();
		RenderCommandPool::init();
		RenderCore::initRenderQUeue();
		oxgui::WidgetManager::init();
		/******************************/

		m_lastFrameTime = Timer::getEpoch() / 1000.0;

		enableVSync(false);
	
		connectSignal(tBuiltinSignals::OnGuiEvent);


		if (ogfx::DebugTools::m_debugWindowEnabled)
		{
			DebugTools::m_timesList.updateEntry(DebugTools::FramerateCalculations);
			DebugTools::m_timesList.get(DebugTools::FramerateCalculations).name = "Framerate Calculations";

			DebugTools::m_timesList.updateEntry(DebugTools::OnSecondsUpdate);
			DebugTools::m_timesList.get(DebugTools::OnSecondsUpdate).name = "Seconds Update";

			DebugTools::m_timesList.updateEntry(DebugTools::EventHandling);
			DebugTools::m_timesList.get(DebugTools::EventHandling).name = "Event Handling";

			DebugTools::m_timesList.updateEntry(DebugTools::OnUpdate);
			DebugTools::m_timesList.get(DebugTools::OnUpdate).name = "Update";

			DebugTools::m_timesList.updateEntry(DebugTools::OnRender);
			DebugTools::m_timesList.get(DebugTools::OnRender).name = "Render";

			DebugTools::m_timesList.updateEntry(DebugTools::OnDisplay);
			DebugTools::m_timesList.get(DebugTools::OnDisplay).name = "Render Queue Execution";

			DebugTools::m_timesList.updateEntry(DebugTools::OnFrameFinalize);
			DebugTools::m_timesList.get(DebugTools::OnFrameFinalize).name = "Frame Finalize";

			DebugTools::m_timesList.updateEntry(DebugTools::OnSignalsRefresh);
			DebugTools::m_timesList.get(DebugTools::OnSignalsRefresh).name = "Signals Refresh";

			DebugTools::m_timesList.updateEntry(DebugTools::OnWindowDisplay);
			DebugTools::m_timesList.get(DebugTools::OnWindowDisplay).name = "Flip Buffers";

			DebugTools::m_timesList.updateEntry(DebugTools::TotalFrameTime);
			DebugTools::m_timesList.get(DebugTools::TotalFrameTime).name = "Total Frame Time";

			m_debugToolsWindow.create({ 5, 5 }, { 600, 410 });
			m_debugToolsWindow.enable();
			m_debugToolsWindow.setTitle("Debug Tools");
			// m_debugToolsWindow.setBounds(5, 5, 500, 385);
			oxgui::WidgetTheme theme = oxgui::WidgetManager::getGlobalTheme();
			oxgui::WidgetManager::addWidget(m_debugToolsWindow);
			theme.subWindow.titleBarColor_focused = { 85, 0, 85 };
			theme.subWindow.backgroundColor = { 0, 0, 0, 235 };
			theme.subWindow.borderColor_focused = { 85, 0, 85 };
			m_debugToolsWindow.setTheme(theme);
			m_debugToolsWindow.enableGlobalThemeOverride(true);
			theme.tabPanel.backgroundColor = { 10, 10, 10 };
			theme.tabPanel.tabBackground_focused = { 10, 10, 10 };
			// theme.tabPanel.tabBarBackground = { 85, 0, 85 };
			DebugTools::m_debugWindowTabPanel.setTheme(theme);
			DebugTools::m_debugWindowTabPanel.enableGlobalThemeOverride(true);

			DebugTools::m_debugWindowTabPanel.create(m_debugToolsWindow);
			DebugTools::m_debugWindowTabPanel.addTab(DebugTools::m_debugWindowInfoTab);
			DebugTools::m_debugWindowTabPanel.addTab(DebugTools::m_debugWindowWatcherTab);
			DebugTools::m_debugWindowTabPanel.addTab(DebugTools::m_debugWindowTab3);
			DebugTools::m_debugWindowInfoTab.setText("Debug Info");
			DebugTools::m_debugWindowWatcherTab.setText("Watcher");
			DebugTools::m_debugWindowTab3.setText("Tab3");
			DebugTools::m_debugWindowTabPanel.selectTab(0);

			DebugTools::m_debugInfoWidget.init(DebugTools::m_debugWindowInfoTab);
		}

		setTypeName("ogfx::GraphicsApplication");
		validate();
		m_window.initialize();
		onSetup();
		return *this;
	}

	void GraphicsApplication::destroy(void)
	{
		RenderCommandPool::destroy();
		Renderer2D::shutdown();
	}

	void GraphicsApplication::nextFrame(void)
	{
		if (!isRunning()) return;

		const bool DEBUG = ogfx::DebugTools::m_debugWindowEnabled;

		if (DEBUG)
		{
			DebugTools::m_totalFrameTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		double currTime = Timer::getEpoch() / 1000.0;
		m_rtfps++;
		RTData::DeltaTime = (float)(currTime - m_lastFrameTime);
		m_lastFrameTime = currTime;
		double timeDiff = currTime - m_prevTime;
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::FramerateCalculations).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::FramerateCalculations).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		if (timeDiff >= 1.0f)
		{
			onSecondsUpdate();
			// SoundHandler::update();
			m_prevTime = currTime;
			if (DEBUG)
			{
				DebugTools::m_fps = m_rtfps;
				DebugTools::m_ups = m_rtups;
				for (auto& data : DebugTools::m_timesList.data)
				{
					data.second.computeAvgTime();
					data.second.sampleCount = 1;
					data.second.totalTime = data.second.getAvgTime();
				}
			}
			m_rtfps = 0;
			m_rtups = 0;
		}
		onFrameStart();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnSecondsUpdate).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnSecondsUpdate).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		m_window.handleEvents();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::EventHandling).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::EventHandling).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		while (m_upsAccumulator > m_upsStep)
		{
			onUpdate();
        	m_upsAccumulator -= m_upsStep;
			m_rtups++;
			if (DEBUG)
			{
				SignalHandler::emitSignal(DebugTools::OnProfileDataRecieved, tSignalPriority::RealTime, DebugTools::m_timesList);
				ogfx::DebugTools::m_drawBasicFrameInfo = ogfx::DebugTools::m_debugWindowTabPanel.getSelectedTab() != 0;
			}
		}
    	m_upsAccumulator += m_upsClock.restart();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnUpdate).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnUpdate).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		onRender();
		onGuiRender();
		Renderer2D::resetStats();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnRender).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnRender).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		RenderCore::display();
		if (DEBUG)
		{
			DebugTools::m_renderCmdCount = RenderCore::getRenderCmdCount();
		}
		RenderCore::resetRenderCmdCount();
		Renderer2D::setDefaultRenderTarget();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnDisplay).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnDisplay).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		SignalHandler::refresh();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnSignalsRefresh).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnSignalsRefresh).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		m_window.renderFrame();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnWindowDisplay).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnWindowDisplay).sampleCount++;
			DebugTools::m_profileTimer.start(false, "", ostd::eTimeUnits::Milliseconds);
		}
		onFrameEnd();
		RenderCommandPool::newFrame();
		if (DEBUG)
		{
			DebugTools::m_uniformUpdates = Shader::getUniformUpdateCount();
		}
		Shader::resetUniformUpdateCount();
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::OnFrameFinalize).totalTime += DebugTools::m_profileTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::OnFrameFinalize).sampleCount++;
		}
		if (!isRunning())
		{
			destroy();
			OX_DEBUG("Destroying SFML Window");
		}
		if (DEBUG)
		{
			DebugTools::m_timesList.get(DebugTools::TotalFrameTime).totalTime += DebugTools::m_totalFrameTimer.end(false);
			DebugTools::m_timesList.get(DebugTools::TotalFrameTime).sampleCount++;
			for (auto& data : DebugTools::m_timesList.data)
			{
				data.second.computeAvgTime();
			}
		}
	}

	void GraphicsApplication::handleSignal(tSignal& signal)
	{
		if (signal.ID == tBuiltinSignals::OnGuiEvent)
		{
			Event& evt = (Event&)signal.userData;
			sf::Event& event = evt.sf();
			if (event.type == sf::Event::Closed)
			{
				m_window.close();
				onClose();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					m_window.close();
					onClose();
				}
				onKeyPressed(evt);
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				onKeyReleased(evt);
			}
			else if (event.type == sf::Event::Resized)
			{
				glViewport(0, 0, event.size.width, event.size.height);
				m_window.initialize();
			}
			else if (event.type == sf::Event::MouseMoved)
			{
				onMousePressed(evt);
			}
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				onMouseReleased(evt);
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				onMouseMoved(evt);
			}
		}

		onSignal(signal);
	}

	void GraphicsApplication::drawDebugInfo(RenderCore& gfx, Color col)
	{
		if (!ogfx::DebugTools::m_debugWindowEnabled || !ogfx::DebugTools::m_drawBasicFrameInfo) return;
		gfx.drawText(StringEditor("FPS: ").addi(ogfx::DebugTools::m_fps).str(), { 5, 5 }, 30.0f, col);
		gfx.drawText(StringEditor("FrameTime: ").addf(DebugTools::m_timesList.get(DebugTools::TotalFrameTime).getAvgTime()).str(), { 5, 35 }, 30.0f, col);
	}
}