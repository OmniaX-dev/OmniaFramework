#include "Widgets.hpp"
#include <ostd/Signals.hpp>
#include "Window.hpp"
#include "RenderCore.hpp"
#include "Utils.hpp"

namespace ogfx
{
	namespace oxgui
	{
		using namespace ostd;

		Widget::Widget(void)
		{
			enableSignals(true);

			setTypeName("ogfx::oxgui::Widget");
			validate();
		}

		void Widget::connectSignals(void)
		{
			connectSignal(tBuiltinSignals::OnGuiEvent);
			connectSignal(tBuiltinSignals::WindowResized);
			connectSignal(OnWidgetFocusGainedSignal);
		}

		void Widget::handleSignal(ostd::tSignal& signal)
		{
			if (!isEnabled()) return;
			if (m_parent != nullptr)
			{
				onSignal(signal);
				return;
			}
			if (signal.ID == tBuiltinSignals::OnGuiEvent)
			{
				Event& evt = static_cast<Event&>(signal.userData);
				sf::Event& event = evt.sf();
				if (event.type == sf::Event::KeyPressed)
				{
					if (!isFocused()) return;
					onKeyPressed(evt);
					signal.handled = true;
				}
				else if (event.type == sf::Event::KeyReleased)
				{
					if (!isFocused()) return;
					onKeyReleased(evt);
					signal.handled = true;
				}
				else if (event.type == sf::Event::MouseButtonPressed)
				{
					Vec2 mousePos { (float)event.mouseButton.x, (float)event.mouseButton.y };
					if (!contains(mousePos, true))
					{
						if (isFocused())
						{
							focus(false);
							onFocusLost();
						}
						return;
					}
					if (!isFocused())
					{
						if (WidgetManager::testForFocus(*this, mousePos))
						{
							focus();
							SignalHandler::emitSignal(OnWidgetFocusGainedSignal, tSignalPriority::RealTime, *this);
							onFocusGained();
							__on_mouse_pressed(evt);
							signal.handled = true;
						}
					}
					else
					{
						__on_mouse_pressed(evt);
						signal.handled = true;
					}
				}
				else if (event.type == sf::Event::MouseButtonReleased)
				{
					Vec2 mousePos { (float)event.mouseButton.x, (float)event.mouseButton.y };
					if (!contains(mousePos, true) || !isFocused()) return;
					onMouseReleased(evt);
					signal.handled = true;
				}
				else if (event.type == sf::Event::MouseMoved)
				{
					Vec2 mousePos { (float)event.mouseMove.x, (float)event.mouseMove.y };
					if (isFocused())
					{
						__on_mouse_moved(evt);
						signal.handled = true;
					}
				}
			}
			else if (signal.ID == OnWidgetFocusGainedSignal)
			{
				auto& sender = (Widget&)signal.userData;
				if (!sender.compareByOID(*this)) focus(false);
			}
			onSignal(signal);
		}

		void Widget::setx(float x)
		{
			for (auto& w : m_widgets)
			{
				float xdiff = w.second->getx() - getx();
				w.second->setx(x + xdiff);
			}
			this->x = x;
		}

		void Widget::sety(float y)
		{
			for (auto& w : m_widgets)
			{
				float ydiff = w.second->gety() - gety();
				w.second->sety(y + ydiff);
			}
			this->y = y;
		}

		WidgetID Widget::addWidget(Widget& widget)
		{
			if (widget.m_parent != nullptr)
				return tWidgetState::WidgetHasParent;
			for (auto& w : m_widgets)
			{
				if (w.second->compareByOID(widget))
					return tWidgetState::DuplicatedWidget;
			}
			m_widgets[s_nextWidgetID++] = &widget;
			widget.m_parent = this;
			widget.addPos(getPosition());
			__recursize_handle_set(*this);
			return s_nextWidgetID - 1;
		}

		void Widget::focus(bool value)
		{
			m_focused = value;
			// if (!value)
			// {
				for (auto& w : m_widgets)
					w.second->focus(false);
			// }
		}

		void Widget::drawOutlinedQuad(ogfx::RenderCore& gfx, const ostd::Vec2& position, const ostd::Vec2& size, const ostd::Color& fill, const ostd::Color& border, float borderWidth)
		{
			Vec2 pos = position + getPosition();
			gfx.drawQuad(pos, size, fill, false);
			gfx.drawQuad(pos, { size.x, borderWidth }, border, false);
			gfx.drawQuad({ pos.x, pos.y + size.y - borderWidth }, { size.x, borderWidth }, border, false);
			gfx.drawQuad({ pos.x, pos.y + borderWidth }, { borderWidth, size.y - (borderWidth * 2) }, border, false);
			gfx.drawQuad({ pos.x + size.x - borderWidth, pos.y + borderWidth }, { borderWidth, size.y - (borderWidth * 2) }, border, false);
		}

		void Widget::render(ogfx::RenderCore& gfx)
		{
			if (!isEnabled()) return;
			// if (!ostd::StringEditor(getTypeName()).startsWith("ogfx::oxgui"))
			// 	return;
			renderFrame(gfx);
			renderContent(gfx);
			if (m_disableAutoChildrenRender)
				return;
			for (auto& w : m_widgets)
				w.second->render(gfx);
		}

		void Widget::__on_mouse_pressed(ogfx::Event& evt)
		{
			for (auto& w : m_widgets)
				w.second->__on_mouse_pressed(evt);
			onMousePressed(evt);
		}

		void Widget::__on_mouse_released(ogfx::Event& evt)
		{
			onMouseReleased(evt);
		}
		
		void Widget::__on_mouse_moved(ogfx::Event& evt)
		{
			for (auto& w : m_widgets)
				w.second->__on_mouse_moved(evt);
			onMouseMoved(evt);
		}
		
		void Widget::__on_key_pressed(ogfx::Event& evt)
		{
			onKeyPressed(evt);
		}
		
		void Widget::__on_key_released(ogfx::Event& evt)
		{
			onKeyReleased(evt);
		}

		void Widget::__recursize_handle_set(Widget& widget)
		{
			widget.setSubWindowHandle(getSubWindowHandle());
			if (widget.getChildren().size() == 0) return;
			for (auto& w : widget.getChildren())
				__recursize_handle_set(*w.second);
		}
		





		void WidgetManager::init(void)
		{
			//SubWindow Theme
			s_globalTheme.subWindow.titleBarColor_focused = { 20, 80, 200 };
			s_globalTheme.subWindow.titleBarColor_unfocused = { 50, 50, 50, 255 };
			s_globalTheme.subWindow.backgroundColor = { 0, 0, 0, 225 };
			s_globalTheme.subWindow.borderColor_focused = { 20, 80, 255 };
			s_globalTheme.subWindow.borderColor_unfocused = { 50, 50, 50, 255 };
			s_globalTheme.subWindow.closeButtonColor = { 120, 30, 50 };
			s_globalTheme.subWindow.closeButtonColor_hover = { 170, 30, 50 };
			s_globalTheme.subWindow.closeButtonTextColor = { 200, 200, 200 };
			s_globalTheme.subWindow.titleColor = { 200, 200, 200 };

			s_globalTheme.subWindow.closeButtonFont = ResourceManager::getDefaultBitmapFont();
			s_globalTheme.subWindow.titleFont = ResourceManager::getDefaultBitmapFont();
			s_globalTheme.subWindow.font = ResourceManager::getDefaultBitmapFont();


			//TabPanel Theme
			s_globalTheme.tabPanel.backgroundColor = { 30, 30, 30, 255 };
			s_globalTheme.tabPanel.tabBackground_focused = { 30, 30, 30, 255 };
			s_globalTheme.tabPanel.tabBackground_unfocused = { 20, 20, 20, 255 };
			s_globalTheme.tabPanel.focusedTabHighlight = { 255, 140, 35, 255 };
			s_globalTheme.tabPanel.unfocusedTabHighlight = { 255, 140, 35, 60 };
			s_globalTheme.tabPanel.tabBarBackground = { 5, 5, 5, 255 };
			s_globalTheme.tabPanel.tabBackground_hover = { 50, 50, 50, 255 };
			s_globalTheme.tabPanel.focusedTabNameColor = { 220, 220, 220, 225 };
			s_globalTheme.tabPanel.unfocusedTabNameColor = { 100, 100, 100, 255 };

			s_globalTheme.tabPanel.tabNameFont = ResourceManager::getDefaultBitmapFont();
			s_globalTheme.tabPanel.font = ResourceManager::getDefaultBitmapFont();
		}

		void WidgetManager::addWidget(Widget& widget)
		{
			int32_t handle = s_widgets.size();
			recursive_handle_set(widget, handle);
			s_widgets.push_back(&widget);
			s_widgetOrder.push_back(handle);
		}

		void WidgetManager::render(RenderCore& gfx)
		{
			// Renderer2D::tTextInfo textInfo;
			// textInfo.font = ResourceManager::getDefaultBitmapFont();
			// textInfo.characterHeight = 30;
			// textInfo.color = { 30, 220, 40 };
			// textInfo.characterSpacing = 0
			reorder_widgets();
			for (auto& index : s_widgetOrder)
			{
				s_widgets[index]->render(gfx);
				// StringEditor str;
				// str.addi(find_index(index));
				// gfx.drawText(str.str(), s_widgets[index]->getPosition() + Vec2 { 5, 5 }, textInfo);
			}
		}

		bool WidgetManager::testForFocus(Widget& widget, const ostd::Vec2& mouseClick)
		{
			if (widget.isFocused()) return true;
			
			for (int32_t i = s_widgetOrder.size() - 1; i >= 0; i--)
			{
				uint32_t index = s_widgetOrder[i];
				if (s_widgets[index]->contains(mouseClick))
				{
					if (s_widgets[index]->compareByOID(widget))
						return true;
					return false;
				}
			}
			return false;
		}

		Widget& WidgetManager::getSubWindow(int32_t handle)
		{
			if (handle < 0 || handle >= s_widgets.size())
				return (Widget&)BaseObject::InvalidRef();
			return *s_widgets[handle];
		}

		int32_t WidgetManager::find_index(uint32_t index)
		{
			for (int32_t i = 0; i < s_widgetOrder.size(); i++)
			{
				if (s_widgetOrder[i] == index) return i;
			}
			return -1;
		}

		void WidgetManager::recursive_handle_set(Widget& widget, int32_t handle)
		{
			widget.setSubWindowHandle(handle);
			if (widget.getChildren().size() == 0) return;
			for (auto& w : widget.getChildren())
				recursive_handle_set(*w.second, handle);
		}

		void WidgetManager::reorder_widgets(void)
		{
			Widget* focused_widget = nullptr;
			int32_t i = 0;
			for (i = s_widgetOrder.size() - 1; i >= 0; i--)
			{
				Widget& w = *s_widgets[s_widgetOrder[i]];
				if (w.isFocused())
				{
					focused_widget = &w;
					break;
				}
			}
			if (focused_widget == nullptr) return;
			if (i == s_widgetOrder.size() - 1) return;
			uint32_t index = s_widgetOrder[i];
			s_widgetOrder.erase(s_widgetOrder.begin() + i);
			s_widgetOrder.push_back(index);
		}





		SubWindow& SubWindow::create(const ostd::Vec2& position, const ostd::Vec2& size)
		{
			setPosition(position);
			setSize(size);
			
			connectSignals();

			m_title = "Window Title";

			setTypeName("ogfx::oxgui::SubWindow");
			validate();
			return *this;
		}

		void SubWindow::renderFrame(ogfx::RenderCore& gfx)
		{
			WidgetTheme* theme = &WidgetManager::getGlobalTheme();
			if (m_overrideGlobalTheme)
				theme = &m_theme;
			float closeBtnX = getx() + getw() - m_closeBtnWidth - 1;
			m_closeBtnRect = Rectangle({ closeBtnX, gety() }, { m_closeBtnWidth, m_titleBarHeight - 3 });
			float border_width = 1;
			Color col;
			if (isFocused()) col = theme->subWindow.borderColor_focused;
			else col = theme->subWindow.borderColor_unfocused;
			if (theme->subWindow.enableShadow)
				gfx.drawImage(ResourceManager::getBasicShadowTexture(), getPosition() - Vec2 { 50, 50 }, getSize() + Vec2 { 100, 100 }, { 255, 255, 255, (uint8_t)(isFocused() ? 255 : 100) }, false);
			drawOutlinedQuad(gfx, { 0, 0 }, getSize(), theme->subWindow.backgroundColor, col, border_width);
			if (isFocused()) col = theme->subWindow.titleBarColor_focused;
			else col = theme->subWindow.titleBarColor_unfocused;
			gfx.drawQuad(getPosition(), { getw(), m_titleBarHeight }, col, false);
			Color closeBtnCol = theme->subWindow.closeButtonColor;
			if (m_hoverCloseBtn)
				closeBtnCol = theme->subWindow.closeButtonColor_hover;
			gfx.drawQuad(m_closeBtnRect.getPosition(), m_closeBtnRect.getSize(), closeBtnCol, false);
			Renderer2D::Text::font = theme->subWindow.closeButtonFont;
			Renderer2D::Text::characterHeight = 20;
			Renderer2D::Text::characterSpacing = 0;
			float closeBtnTxtw = Renderer2D::Text::getStringBounds("X").x;
			gfx.drawText("X", Vec2 { closeBtnX + ((m_closeBtnWidth / 2.0f) - (closeBtnTxtw / 2.0f)), gety() - 2 }, theme->subWindow.closeButtonFont, (float)20, theme->subWindow.closeButtonTextColor);
			Vec2 titleBounds = Renderer2D::Text::getStringBounds(m_title);
			Renderer2D::Text::font = theme->subWindow.titleFont;
			gfx.drawText(m_title, Vec2 { getx() + ((getw() / 2.0f) - (titleBounds.x / 2.0f)) - (m_closeBtnWidth / 2.0f), gety() }, theme->subWindow.titleFont, (float)20, theme->subWindow.titleColor);
		}

		void SubWindow::onMousePressed(ogfx::Event& evt)
		{
			auto& sfevt = evt.sf();
			Vec2 mpos = { (float)sfevt.mouseButton.x, (float)sfevt.mouseButton.y };
			if (Rectangle(getPosition(), { getw(), m_titleBarHeight }).contains(mpos, false))
			{
				m_mousePressed = true;
				m_clickPos = mpos;
			}
		}

		void SubWindow::onMouseReleased(ogfx::Event& evt)
		{
			m_mousePressed = false;
		}

		void SubWindow::onMouseMoved(ogfx::Event& evt)
		{
			auto& sfevt = evt.sf();
			Vec2 mpos = { (float)sfevt.mouseMove.x, (float)sfevt.mouseMove.y };
			if (m_closeBtnRect.contains(mpos, true))
				m_hoverCloseBtn = true;
			else
				m_hoverCloseBtn = false;
			if (!isFocused() && !m_mousePressed) return;
			if (m_mousePressed && contains(mpos, true))
			{
				Vec2 diff = mpos - m_clickPos;
				addPos(diff);
				m_clickPos = mpos;
			}
			else if (m_mousePressed)
				m_mousePressed = false;
		}

		void SubWindow::onSignal(tSignal& signal)
		{
			if (signal.ID == tBuiltinSignals::WindowResized)
			{
				WindowSizeObj& wsobj = static_cast<WindowSizeObj&>(signal.userData);
				if (getx() + getw() > wsobj.width)
				{
					float xdiff = wsobj.width - getx();
					xdiff = getw() - xdiff;
					subx(xdiff);
				}
				if (gety() + geth() > wsobj.height)
				{
					float ydiff = wsobj.height - gety();
					ydiff = geth() - ydiff;
					suby(ydiff);
				}
			}
		}






		TabWidget& TabWidget::create(TabPanel& parent, const ostd::String& text)
		{
			if (parent.isInvalid())
			{
				invalidate();
				return *this;
			}
			m_text = text;

			setPosition({ 0, 5 });
			setSize(parent.getContentSize());

			m_disableAutoChildrenRender = true;

			setTypeName("ogfx::oxgui::TabWidget");
			validate();
			enable();
			parent.addWidget(*this);
			return *this;
		}

		void TabWidget::onMousePressed(ogfx::Event& evt)
		{

		}
		
		void TabWidget::onMouseReleased(ogfx::Event& evt)
		{
			
		}





		TabPanel& TabPanel::create(Widget& parent, const ostd::Vec2& subPosition, const ostd::Vec2& size)
		{
			if (parent.isInvalid())
			{
				invalidate();
				return *this;
			}
			setPosition(parent.getContentOffset() + subPosition);
			if (size == Vec2 { 0, 0 })
				setSize(parent.getContentSize());
			else
				setSize(size);

			m_enableTabBarColorBlend = true;

			setTypeName("ogfx::oxgui::TabPanel");
			connectSignals();
			validate();
			enable();
			parent.addWidget(*this);
			return *this;
		}

		void TabPanel::renderFrame(ogfx::RenderCore& gfx)
		{
			WidgetTheme* theme = &WidgetManager::getGlobalTheme();
			if (m_overrideGlobalTheme)
				theme = &m_theme;

			drawOutlinedQuad(gfx, { 0, 0 }, getSize(), theme->tabPanel.backgroundColor, theme->tabPanel.backgroundColor);
			
			Color tabBarColor = theme->tabPanel.tabBarBackground;
			if (m_enableTabBarColorBlend)
			{
				Widget& baseWin = WidgetManager::getSubWindow(getSubWindowHandle());
				if (baseWin.isValid())
				{
					if (baseWin.isFocused())
						tabBarColor = theme->subWindow.titleBarColor_focused;
					else
						tabBarColor = theme->subWindow.titleBarColor_unfocused;
				}
			}

			gfx.drawQuad(getPosition() - Vec2 { 0, 1 }, { getw(), theme->tabPanel.tabBarHeight }, tabBarColor, false);

			if (m_selectedTab < 0) return;

			Renderer2D::tTextInfo textInfo;
			textInfo.characterHeight = theme->tabPanel.tabNameFontSize;
			textInfo.characterSpacing = 3;
			textInfo.font = theme->tabPanel.tabNameFont;
			Renderer2D::Text::characterHeight = textInfo.characterHeight;
			Renderer2D::Text::characterSpacing = textInfo.characterSpacing;
			Renderer2D::Text::font = textInfo.font;


			float tabx = 0;
			m_hoverTab = -1;
			for (uint32_t i = 0; i < m_tabs.size(); i++)
			{
				auto& tab = m_tabs[i];
				float tabw = theme->tabPanel.fixedTabWidth;
				Vec2 tabPos = getPosition() + Vec2 { tabx, 0 };
				Vec2 tabSize = { tabw, theme->tabPanel.tabBarHeight };
				Color col = theme->tabPanel.tabBackground_unfocused;

				if (i == m_selectedTab)
					col = theme->tabPanel.tabBackground_focused;
				else if (Rectangle(tabPos, tabSize).contains(m_mousePos, true))
				{
					m_hoverTab = i;
					col = theme->tabPanel.tabBackground_hover;
				}
				gfx.drawQuad(tabPos, tabSize, col, false);
				if (i == m_selectedTab)
				{
					gfx.drawQuad(getPosition() + Vec2 { tabx, 0 }, { tabw, theme->tabPanel.tabHighlightHeight }, theme->tabPanel.focusedTabHighlight, false);
					gfx.drawQuad(tabPos, { theme->tabPanel.tabHighlightHeight, tabSize.y }, theme->tabPanel.focusedTabHighlight, false);
					gfx.drawQuad(tabPos + Vec2 { tabw - theme->tabPanel.tabHighlightHeight, 0 }, { theme->tabPanel.tabHighlightHeight, tabSize.y }, theme->tabPanel.focusedTabHighlight, false);
				}
				else
				{
					gfx.drawQuad(getPosition() + Vec2 { tabx, theme->tabPanel.tabBarHeight - theme->tabPanel.tabHighlightHeight }, { tabw, theme->tabPanel.tabHighlightHeight }, theme->tabPanel.focusedTabHighlight, false);

					gfx.drawQuad(tabPos, { theme->tabPanel.tabHighlightHeight, tabSize.y - theme->tabPanel.tabHighlightHeight }, { 50, 50, 50 }, false);
				}
				Vec2 nameBounds = Renderer2D::Text::getStringBounds(tab->getText());
				if (i == m_selectedTab)
					textInfo.color = theme->tabPanel.focusedTabNameColor;
				else
					textInfo.color = theme->tabPanel.unfocusedTabNameColor;
				gfx.drawText(tab->getText(), getPosition() + Vec2 { tabx, 0 } + ((Vec2 { tabw, 0 } / 2.0f) - Vec2 { nameBounds.x / 2.0f, -2 }), textInfo);
				tabx += tabw;
			}
		}

		void TabPanel::renderContent(ogfx::RenderCore& gfx)
		{
			if (m_selectedTab == -1) return;
			auto& tab = m_tabs[m_selectedTab];

			for (auto& w : tab->getChildren())
				w.second->render(gfx);
		}

		ostd::Vec2 TabPanel::getContentSize(void)
		{
			float tabBarHeight = WidgetManager::getGlobalTheme().tabPanel.tabBarHeight;
			if (isGlobalThemeOverrideEnabled())
				tabBarHeight = m_theme.tabPanel.tabBarHeight;
			return getSize() - ostd::Vec2 { 0, tabBarHeight };
		}

		ostd::Vec2 TabPanel::getContentOffset(void)
		{
			float tabBarHeight = WidgetManager::getGlobalTheme().tabPanel.tabBarHeight;
			if (isGlobalThemeOverrideEnabled())
				tabBarHeight = m_theme.tabPanel.tabBarHeight;
			return ostd::Vec2 { 0, tabBarHeight };
		}

		void TabPanel::onMousePressed(ogfx::Event& evt)
		{
			if (m_hoverTab != -1)
			{
				m_selectedTab = m_hoverTab;
				m_hoverTab = -1;
			}
		}

		void TabPanel::onMouseReleased(ogfx::Event& evt)
		{

		}


		void TabPanel::onMouseMoved(ogfx::Event& evt)
		{
			auto& sfmpos = evt.sf().mouseMove;
			m_mousePos = { (float)sfmpos.x, (float)sfmpos.y };
		}

		// TabWidget& TabPanel::newTab(const ostd::String& text)
		// {
		// 	TabWidget tab;
		// 	m_tabs.push_back(tab);
		// 	m_tabs[m_tabs.size() - 1].create(*this, text);
		// 	m_selectedTab = m_tabs.size() - 1;
		// 	return m_tabs[m_tabs.size() - 1];
		// }

		void TabPanel::addTab(TabWidget& tab)
		{
			tab.create(*this, "");
			m_tabs.push_back(&tab);
			m_selectedTab = m_tabs.size() - 1;
		}
	}
}