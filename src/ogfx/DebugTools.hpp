#pragma once

#include <ostd/Utils.hpp>
#include <unordered_map>
#include <ostd/BaseObject.hpp>
#include <ogfx/RTData.hpp>
#include <ogfx/Widgets.hpp>

namespace ogfx
{
	class DebugTimeEntry
	{
		public:
			ostd::String name { "UNNAMED" };
			double totalTime { 0 };
			uint32_t sampleCount { 0 };

			double avgTime { 0 };

			inline double getAvgTime(void)
			{
				return avgTime;
			}

			inline void computeAvgTime(void)
			{
				if (sampleCount <= 0) return;
				avgTime =  totalTime / sampleCount;
			}
	};

	class DebugTimesList : public ostd::BaseObject
	{
		public:
			std::unordered_map<uint32_t, DebugTimeEntry> data;

			inline void updateEntry(uint32_t id, DebugTimeEntry entry = DebugTimeEntry()) { data[id] = entry; }
			inline DebugTimeEntry& get(uint32_t id) { return data[id]; }
	};

	class DebugInfoWidget : public ogfx::oxgui::Widget
	{
		public:
			inline DebugInfoWidget(void) { }
			void init(ogfx::oxgui::Widget& parent);

			void renderContent(ogfx::RenderCore& gfx) override;
			void onSignal(ostd::tSignal& signal) override;

		private:
			DebugTimesList* m_dataList { nullptr };
	};

	class GraphicsApplication2D;
	class DebugTools
	{
		public:
			static void showDebugWindow(GraphicsApplication2D* parent_app, bool show = true);

			inline static ostd::Timer m_profileTimer;
			inline static ostd::Timer m_totalFrameTimer;
			inline static DebugTimesList m_timesList;

			inline static uint32_t m_fps { 0 };
			inline static uint32_t m_ups { 0 };

			inline static uint32_t m_uniformUpdates { 0 };
			inline static uint32_t m_renderCmdCount { 0 };

			inline static bool m_debugWindowEnabled { false };
			inline static bool m_drawBasicFrameInfo { false };

			inline static DebugInfoWidget m_debugInfoWidget;
			inline static ogfx::oxgui::TabPanel m_debugWindowTabPanel;
			inline static ogfx::oxgui::TabWidget m_debugWindowInfoTab;
			inline static ogfx::oxgui::TabWidget m_debugWindowWatcherTab;
			inline static ogfx::oxgui::TabWidget m_debugWindowTab3;

			inline static constexpr uint32_t FramerateCalculations = 0;
			inline static constexpr uint32_t OnSecondsUpdate = 1;
			inline static constexpr uint32_t EventHandling = 2;
			inline static constexpr uint32_t OnSignalsRefresh = 23;
			inline static constexpr uint32_t OnUpdate = 4;
			inline static constexpr uint32_t OnRender = 5;
			inline static constexpr uint32_t OnDisplay = 6;
			inline static constexpr uint32_t OnFrameFinalize = 7;
			inline static constexpr uint32_t OnWindowDisplay = 8;
			inline static constexpr uint32_t TotalFrameTime = 128;

			inline static const uint32_t OnProfileDataRecieved = ogfx::RTData::newCustomSignal(2200);

			inline static void enableDebugWindow(bool enable = true) { m_debugWindowEnabled = enable; }
	};
}