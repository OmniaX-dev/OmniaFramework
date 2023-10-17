#pragma once

#include <ostd/BaseObject.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Color.hpp>
#include <ogfx/RTData.hpp>

#include <unordered_map>

namespace ogfx
{
	class RenderCore;
	class Event;
	
	namespace oxgui
	{
		struct tWidgetState
		{
			inline static constexpr ostd::WidgetID DuplicatedWidget = 0x00000001; 
			inline static constexpr ostd::WidgetID WidgetHasParent = 0x00000002;

			inline static constexpr ostd::WidgetID ValidWidgetBase = 0x00000400;

			inline static bool check(ostd::WidgetID result) { return result >= ValidWidgetBase; }
		};

		class WidgetTheme
		{
			using Col = ostd::Color;
			using Font = ostd::ResourceID;
			public: struct tSubWindowTheme {
				Col titleBarColor_focused;
				Col titleBarColor_unfocused;
				Col titleColor;
				Col closeButtonColor;
				Col closeButtonColor_hover;
				Col closeButtonTextColor;
				Col borderColor_focused;
				Col borderColor_unfocused;
				Col backgroundColor;

				Font titleFont { 0 };
				Font closeButtonFont { 0 };
				Font font { 0 };

				bool enableShadow { true };
			};

			public: struct tTabPanel {
				Col backgroundColor;
				Col tabBackground_focused;
				Col tabBackground_unfocused;
				Col focusedTabHighlight;
				Col unfocusedTabHighlight;
				Col tabBarBackground;
				Col tabBackground_hover;
				Col focusedTabNameColor;
				Col unfocusedTabNameColor;

				Font tabNameFont { 0 };
				Font font { 0 };

				float fixedTabWidth { 130 };
				float tabBarHeight { 25 };
				float tabHighlightHeight { 1 };
				float tabNameFontSize { 20 };
			};

			public:
				tSubWindowTheme subWindow;
				tTabPanel tabPanel;
		};

		class Widget : public ostd::BaseObject, public ostd::Rectangle
		{
			public:
				~Widget(void) = default;
				void connectSignals(void);
				void handleSignal(ostd::tSignal& signal) override;

				inline void enableGlobalThemeOverride(bool enable = true) { m_overrideGlobalTheme = enable; }
				inline bool isGlobalThemeOverrideEnabled(void) { return m_overrideGlobalTheme; }
				inline void setTheme(WidgetTheme theme) { m_theme = theme; }
				inline WidgetTheme& getTheme(void) { return m_theme; }
				inline std::unordered_map<ostd::WidgetID, Widget*>& getChildren(void) { return m_widgets; }
				inline void setSubWindowHandle(int32_t handle) { m_subWindowHandle = handle; }
				inline int32_t getSubWindowHandle(void) { return m_subWindowHandle; }

				void setx(float x) override;
				void sety(float y) override;

				ostd::WidgetID addWidget(Widget& widget);

				inline void enable(bool enabled = true) { m_enabled = enabled; }
				inline bool isEnabled(void) { return m_enabled; }
				void focus(bool value = true);
				inline bool isFocused(void) { return m_focused; }

				inline virtual ostd::Vec2 getContentSize(void) { return getSize(); }
				inline virtual ostd::Vec2 getContentOffset(void) { return { 0, 0 }; }

				inline virtual void onSignal(ostd::tSignal& signal) {  }
				inline virtual void onKeyPressed(ogfx::Event& evt) {  }
				inline virtual void onKeyReleased(ogfx::Event& evt) {  }
				inline virtual void onMousePressed(ogfx::Event& evt) {  }
				inline virtual void onMouseReleased(ogfx::Event& evt) {  }
				inline virtual void onMouseMoved(ogfx::Event& evt) {  }
				inline virtual void onFocusGained(void) {  }
				inline virtual void onFocusLost(void) {  }

				inline virtual void renderFrame(ogfx::RenderCore& gfx) {  }
				inline virtual void renderContent(ogfx::RenderCore& gfx) {  }

				void render(ogfx::RenderCore& gfx);

			private:
				void __on_mouse_pressed(ogfx::Event& evt);
				void __on_mouse_released(ogfx::Event& evt);
				void __on_mouse_moved(ogfx::Event& evt);
				void __on_key_pressed(ogfx::Event& evt);
				void __on_key_released(ogfx::Event& evt);

				void __recursize_handle_set(Widget& widget);

			protected:
				Widget(void);
				void drawOutlinedQuad(ogfx::RenderCore& gfx, const ostd::Vec2& position, const ostd::Vec2& size, const ostd::Color& fill, const ostd::Color& border, float borderWidth = 2);

			private:
				bool m_enabled { false };
				bool m_focused { false };
				int32_t m_subWindowHandle { -1 };

				std::unordered_map<ostd::WidgetID, Widget*> m_widgets;

				inline static ostd::WidgetID s_nextWidgetID = tWidgetState::ValidWidgetBase;

			protected:
				inline static const uint32_t OnWidgetFocusGainedSignal = RTData::newCustomSignal(1200);
				WidgetTheme m_theme;
				bool m_overrideGlobalTheme { false };
				Widget* m_parent { nullptr };
				bool m_disableAutoChildrenRender { false };
		};

		class WidgetManager : public ostd::BaseObject
		{
			public:
				static void init(void);
				static void addWidget(Widget& widget);
				static void render(RenderCore& gfx);
				static bool testForFocus(Widget& widget, const ostd::Vec2& mouseClick);
				static Widget& getSubWindow(int32_t handle);

				inline static void setGlobalTheme(WidgetTheme theme) { s_globalTheme = theme; }
				inline static WidgetTheme& getGlobalTheme(void) { return s_globalTheme; }

			private:
				static void reorder_widgets(void);
				static int32_t find_index(uint32_t index);
				static void recursive_handle_set(Widget& widget, int32_t handle);

			private:
				inline static std::vector<Widget*> s_widgets;
				inline static std::vector<uint32_t> s_widgetOrder;
				inline static WidgetTheme s_globalTheme;
		};

		class SubWindow : public Widget
		{
			public:
				inline SubWindow(void) : Widget() { invalidate(); }
				inline SubWindow(const ostd::Vec2& position, const ostd::Vec2& size) : Widget() { create(position, size); }
				virtual ~SubWindow(void) = default;
				SubWindow& create(const ostd::Vec2& position, const ostd::Vec2& size);
				inline ostd::String getTitle(void) { return m_title; }
				inline void setTitle(const ostd::String& title) { m_title = title; }
				inline ostd::Vec2 getContentSize(void) override { return getSize() - ostd::Vec2 { m_borderWidth * 2, m_titleBarHeight + (m_borderWidth * 2) }; }
				inline ostd::Vec2 getContentOffset(void) override { return ostd::Vec2 { m_borderWidth, m_titleBarHeight + m_borderWidth }; }
				
				virtual void renderFrame(ogfx::RenderCore& gfx) override;

				void onMousePressed(ogfx::Event& evt) override;
				void onMouseReleased(ogfx::Event& evt) override;
				void onMouseMoved(ogfx::Event& evt) override;

				void onSignal(ostd::tSignal& signal) override;

				// inline virtual void onSignal(ostd::tSignal& signal) {  }
				// inline virtual void onFocusGained(void) {  }
				// inline virtual void onFocusLost(void) {  }
				// void onKeyPressed(ogfx::Event& evt) override;
				// void onKeyReleased(ogfx::Event& evt) override;

			private:
				bool m_mousePressed { false };
				ostd::Vec2 m_clickPos { 0, 0 };
				float m_titleBarHeight { 20 };
				float m_closeBtnWidth { 24 };
				float m_borderWidth { 1 };
				Rectangle m_closeBtnRect;
				bool m_hoverCloseBtn  { false };
				ostd::String m_title { "" };
		};

		class TabPanel;
		class TabWidget : public Widget
		{
			public:
				inline TabWidget(void) : Widget() { invalidate(); }
				inline TabWidget(TabPanel& parent, const ostd::String& text) { create(parent, text); }
				TabWidget& create(TabPanel& parent, const ostd::String& text);

				void onMousePressed(ogfx::Event& evt) override;
				void onMouseReleased(ogfx::Event& evt) override;
				
				inline ostd::Vec2 getContentOffset(void) override { return m_parent->getContentOffset(); }
				inline ostd::Vec2 getContentSize(void) override { return getSize(); }

				inline void setText(const ostd::String& text) { m_text = text; }
				inline ostd::String getText(void) { return m_text; }

			private:
				ostd::String m_text { "" };
		};

		class TabPanel : public Widget
		{
			public:
				inline TabPanel(void) : Widget() { invalidate(); }
				inline TabPanel(Widget& parent, const ostd::Vec2& subPosition = { 0, 0 }, const ostd::Vec2& size = { 0, 0 }) { create(parent, subPosition, size); }
				TabPanel& create(Widget& parent, const ostd::Vec2& subPosition = { 0, 0 }, const ostd::Vec2& size = { 0, 0 });

				virtual void renderFrame(ogfx::RenderCore& gfx) override;
				virtual void renderContent(ogfx::RenderCore& gfx) override;

				ostd::Vec2 getContentSize(void) override;
				ostd::Vec2 getContentOffset(void) override;

				void onMousePressed(ogfx::Event& evt) override;
				void onMouseReleased(ogfx::Event& evt) override;
				void onMouseMoved(ogfx::Event& evt) override;

				// TabWidget& newTab(const ostd::String& text);
				void addTab(TabWidget& tab);
				inline bool selectTab(uint32_t tabIndex) { if (tabIndex >= m_tabs.size()) return false; m_selectedTab = tabIndex; return true; }
				inline int32_t getSelectedTab(void) { return m_selectedTab; }

			private:
				std::vector<TabWidget*> m_tabs;
				int32_t m_selectedTab { -1 };
				ostd::Vec2 m_mousePos { -10000, -10000 };
				int32_t m_hoverTab { -1 };
				bool m_enableTabBarColorBlend { false };
		};
	}
}