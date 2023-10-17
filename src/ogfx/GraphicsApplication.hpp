#pragma once

#include <ostd/BaseObject.hpp>
#include <ogfx/DataStructures.hpp>
#include <ogfx/Window.hpp>
#include <ogfx/DebugTools.hpp>

namespace ogfx
{
	class RenderCore;
	class GraphicsApplication2D : public ostd::BaseObject
	{
		public:
			inline GraphicsApplication2D(void) { invalidate(); }
			inline GraphicsApplication2D(int32_t windowWidth, int32_t windowHeight, const ostd::String& windowTitle, tContextSettings contextSettings = tContextSettings()) { create(windowWidth, windowHeight, windowTitle, contextSettings); }
			GraphicsApplication2D& create(int32_t windowWidth, int32_t windowHeight, const ostd::String& windowTitle, tContextSettings contextSettings = tContextSettings());

			void destroy(void);
			void nextFrame(void);

			inline bool isRunning(void) { return isValid() && m_window.isRunning(); }
			// inline uint32_t& getFps(void) { return m_fps; }
			inline void enableVSync(bool enabled = true) { m_window.sf().setVerticalSyncEnabled(enabled); }
			inline IPoint getWindowSize(void) { return { m_window.getSize().x, m_window.getSize().y }; }
			inline int32_t getWindowWidth(void) { return m_window.getSize().x; }
			inline int32_t getWindowHeight(void) { return m_window.getSize().y; }
			// inline void enableDebugInfo(bool v = true) { m_showDebugInfo = v; }
			// inline bool isDebugInfoEnabled(void) { return m_showDebugInfo; }

			// void drawDebugInfo(RenderCore& gfx, Color col = { 150, 180, 130 });
			void drawDebugInfo(RenderCore& gfx, Color col = { 150, 180, 130 });
			void handleSignal(tSignal& signal) override;

			virtual void onFrameStart(void) {  }
			virtual void onFrameEnd(void) {  }
			virtual void onSetup(void) {  }
			virtual void onRender(void) {  }
			virtual void onUpdate(void) {  }
			virtual void onSecondsUpdate(void) {  }
			virtual void onGuiRender(void) {  }
			virtual void onKeyPressed(Event& evt) {  }
			virtual void onKeyReleased(Event& evt) {  }
			virtual void onMousePressed(Event& evt) {  }
			virtual void onMouseReleased(Event& evt) {  }
			virtual void onMouseMoved(Event& evt) {  }
			virtual void onSignal(tSignal& signal) {  }
			virtual void onClose(void) {  }

		protected:
			Window m_window;
			Color m_clearColor;

		private:
			uint32_t m_rtfps { 0 };
			uint32_t m_rtups { 0 };
			// uint32_t m_fps { 0 };
			// uint32_t m_ups { 0 };
			double m_prevTime { 0.0 };
			double m_lastFrameTime { 0.0 };

			sf::Clock m_upsClock;
			sf::Time m_upsAccumulator = sf::Time::Zero;
			sf::Time m_upsStep = sf::seconds(1.f / 60.f);

			// uint32_t m_uniformUpdates { 0 };
			// uint32_t m_renderCmdCount { 0 };

			// bool m_showDebugInfo { false };

			ogfx::oxgui::SubWindow m_debugToolsWindow;

		public:
			inline static constexpr int32_t ERR_WINDOW_CREATE_FAIL = OX_GFX_APPLICATION_2D_ERR_MASK + 0x0001;
			inline static constexpr int32_t ERR_INVALID_APP_INSTANCE = OX_GFX_APPLICATION_2D_ERR_MASK + 0x0002;


		friend class DebugTools;
	};

	class GraphicsApplication : public ostd::BaseObject
	{
		public:
			inline GraphicsApplication(void) { invalidate(); }
			inline GraphicsApplication(int32_t windowWidth, int32_t windowHeight, const ostd::String& windowTitle, tContextSettings contextSettings = tContextSettings()) { create(windowWidth, windowHeight, windowTitle, contextSettings); }
			GraphicsApplication& create(int32_t windowWidth, int32_t windowHeight, const ostd::String& windowTitle, tContextSettings contextSettings = tContextSettings());

			void destroy(void);
			void nextFrame(void);

			inline bool isRunning(void) { return isValid() && m_window.isRunning(); }
			inline void enableVSync(bool enabled = true) { m_window.sf().setVerticalSyncEnabled(enabled); }
			inline IPoint getWindowSize(void) { return { m_window.getSize().x, m_window.getSize().y }; }
			inline int32_t getWindowWidth(void) { return m_window.getSize().x; }
			inline int32_t getWindowHeight(void) { return m_window.getSize().y; }

			void drawDebugInfo(RenderCore& gfx, Color col = { 150, 180, 130 });
			void handleSignal(tSignal& signal) override;

			virtual void onFrameStart(void) {  }
			virtual void onFrameEnd(void) {  }
			virtual void onSetup(void) {  }
			virtual void onRender(void) {  }
			virtual void onUpdate(void) {  }
			virtual void onSecondsUpdate(void) {  }
			virtual void onGuiRender(void) {  }
			virtual void onKeyPressed(Event& evt) {  }
			virtual void onKeyReleased(Event& evt) {  }
			virtual void onMousePressed(Event& evt) {  }
			virtual void onMouseReleased(Event& evt) {  }
			virtual void onMouseMoved(Event& evt) {  }
			virtual void onSignal(tSignal& signal) {  }
			virtual void onClose(void) {  }

		protected:
			Window m_window;
			Color m_clearColor;

		private:
			uint32_t m_rtfps { 0 };
			uint32_t m_rtups { 0 };
			double m_prevTime { 0.0 };
			double m_lastFrameTime { 0.0 };

			sf::Clock m_upsClock;
			sf::Time m_upsAccumulator = sf::Time::Zero;
			sf::Time m_upsStep = sf::seconds(1.f / 60.f);

			ogfx::oxgui::SubWindow m_debugToolsWindow;

		public:
			inline static constexpr int32_t ERR_WINDOW_CREATE_FAIL = OX_GFX_APPLICATION_2D_ERR_MASK + 0x0001;
			inline static constexpr int32_t ERR_INVALID_APP_INSTANCE = OX_GFX_APPLICATION_2D_ERR_MASK + 0x0002;


		friend class DebugTools;
	};
}