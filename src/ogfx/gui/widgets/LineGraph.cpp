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

#include "LineGraph.hpp"
#include "../../render/BasicRenderer.hpp"
#include "../Window.hpp"
#include "../../../ostd/io/Logger.hpp"
#include <algorithm>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		LineGraph& LineGraph::create(void)
		{
			setPadding({ 0, 0, 0, 0 });
			setTypeName("ogfx::gui::LineGraph");
			disableChildren();
			enableStopEvents();
			enableBackground();
			enableBorder();
			setBackgroundColor({ 20, 20, 20 });
			setBorderColor({ 50, 50, 50 });
			setStylesheetCategoryName("lineGraph");
			reloadTheme();
			validate();
			return *this;
		}

		void LineGraph::applyTheme(const ostd::Stylesheet& theme)
		{
			setPlotBackgroundColor(getThemeValue<Color>(theme, "plotBackgroundColor", getPlotBackgroundColor()));
			setGridColor(getThemeValue<Color>(theme, "gridColor", getGridColor()));
			enableGrid(getThemeValue<bool>(theme, "showGrid", isGridEnabled()));
			setAxisLineColor(getThemeValue<Color>(theme, "axisLineColor", getAxisLineColor()));
			setAxisLabelColor(getThemeValue<Color>(theme, "axisLabelColor", getAxisLabelColor()));
			setAxisTitleColor(getThemeValue<Color>(theme, "axisTitleColor", getAxisTitleColor()));
			setPointRadius(getThemeValue<f32>(theme, "pointRadius", getPointRadius()));
			setLineThickness(getThemeValue<i32>(theme, "lineThickness", getLineThickness()));
			enableLegend(getThemeValue<bool>(theme, "showLegend", isLegendEnabled()));
			setLegendBackgroundColor(getThemeValue<Color>(theme, "legend.backgroundColor", getLegendBackgroundColor()));
			setLegendTextColor(getThemeValue<Color>(theme, "legend.textColor", getLegendTextColor()));
			setLegendHoverTextColor(getThemeValue<Color>(theme, "legend.hoverTextColor", getLegendHoverTextColor()));
			setLegendBorderColor(getThemeValue<Color>(theme, "legend.borderColor", getLegendBorderColor()));
			setCrosshairColor(getThemeValue<Color>(theme, "hover.crosshairColor", getCrosshairColor()));
			setHoverLabelBackgroundColor(getThemeValue<Color>(theme, "hover.labelBackgroundColor", getHoverLabelBackgroundColor()));
			setHoverLabelTextColor(getThemeValue<Color>(theme, "hover.labelTextColor", getHoverLabelTextColor()));
			setHoverCircleColor(getThemeValue<Color>(theme, "hover.circleColor", getHoverCircleColor()));
		}

		void LineGraph::setup(const stdvec<std::pair<String, f64>>& x_axis, const stdvec<std::pair<String, f64>>& y_axis)
		{
			m_xTicks.clear();
			for (auto& p : x_axis)
				m_xTicks.push_back({ p.first, p.second });
			m_yTicks.clear();
			for (auto& p : y_axis)
				m_yTicks.push_back({ p.first, p.second });
			m_xAxisDirty = true;
			m_yAxisDirty = true;
		}

		u32 LineGraph::plot(const stdvec<f64>& xdata, const stdvec<f64>& ydata, const String& name, const Color& color, bool showPoints, const stdvec<ostd::Object*>& userData)
		{
			if (xdata.size() != ydata.size())
			{
				OX_WARN("LineGraph: plot() xdata/ydata size mismatch (%d vs %d).", (i32)xdata.size(), (i32)ydata.size());
				return (u32)-1;
			}
			if (!userData.empty() && userData.size() != xdata.size())
			{
				OX_WARN("LineGraph: plot() userData size (%d) must be empty or match xdata/ydata size (%d).", (i32)userData.size(), (i32)xdata.size());
				return (u32)-1;
			}
			Color requestedColor = color;  // Color::operator== isn't const-qualified, so copy before comparing
			Series s;
			s.name = name;
			s.color = (requestedColor == Colors::Transparent) ? default_palette_color(m_nextPaletteIndex++) : requestedColor;
			s.showPoints = showPoints;
			s.xdata = xdata;
			s.ydata = ydata;
			s.userData = userData;
			m_series.push_back(std::move(s));
			m_xAxisDirty = true;
			m_yAxisDirty = true;
			m_legendCornerDirty = true;
			return (u32)m_series.size() - 1;
		}

		bool LineGraph::removeSeries(u32 index)
		{
			if (index >= m_series.size())
				return false;
			m_series.erase(m_series.begin() + index);
			if (m_hoverSeriesIndex == (i32)index)
			{
				m_hoverSeriesIndex = -1;
				m_hoverPointIndex = -1;
			}
			else if (m_hoverSeriesIndex > (i32)index)
				m_hoverSeriesIndex--;
			if (m_legendHoverIndex == (i32)index)
				m_legendHoverIndex = -1;
			else if (m_legendHoverIndex > (i32)index)
				m_legendHoverIndex--;
			m_xAxisDirty = true;
			m_yAxisDirty = true;
			m_legendCornerDirty = true;
			return true;
		}

		bool LineGraph::removeSeries(const String& name)
		{
			for (u32 i = 0; i < m_series.size(); i++)
			{
				if (m_series[i].name == name)
					return removeSeries(i);
			}
			return false;
		}

		void LineGraph::clearSeries(void)
		{
			m_series.clear();
			m_hoverSeriesIndex = -1;
			m_hoverPointIndex = -1;
			m_legendHoverIndex = -1;
			set_legend_cursor(false);
			m_nextPaletteIndex = 0;
			m_xAxisDirty = true;
			m_yAxisDirty = true;
			m_legendCornerDirty = true;
		}

		LineGraph::Series& LineGraph::getSeries(u32 index)
		{
			if (index < m_series.size())
				return m_series[index];
			return m_invalidSeries;
		}

		const LineGraph::Series& LineGraph::getSeries(u32 index) const
		{
			if (index < m_series.size())
				return m_series[index];
			return m_invalidSeries;
		}

		Color LineGraph::default_palette_color(u32 index)
		{
			static const Color palette[] = {
				Color { 47, 176, 0 },     // green
				Color { 220, 20, 60 },    // crimson
				Color { 0, 150, 220 },    // sky blue
				Color { 255, 165, 0 },    // orange
				Color { 170, 90, 220 },   // purple
				Color { 230, 210, 40 },   // yellow
				Color { 0, 200, 170 },    // teal
				Color { 230, 120, 180 },  // pink
			};
			return palette[index % (u32)(sizeof(palette) / sizeof(palette[0]))];
		}

		Color LineGraph::dim_color(const Color& c)
		{
			// Blend toward mid-gray for a "grayed out" look, independent of the legend background.
			u8 r = (u8)(((i32)c.r + 130) / 2);
			u8 g = (u8)(((i32)c.g + 130) / 2);
			u8 b = (u8)(((i32)c.b + 130) / 2);
			return Color { r, g, b, (u8)c.a };
		}

		void LineGraph::set_legend_cursor(bool active)
		{
			if (active == m_legendCursorActive)
				return;
			getWindow().setCursor(active ? ogfx::WindowCore::eCursor::Pointer : ogfx::WindowCore::eCursor::Default);
			m_legendCursorActive = active;
		}

		LineGraph::AxisRange LineGraph::compute_data_range(bool forXAxis) const
		{
			AxisRange r;
			bool any = false;
			f64 lo = 0.0, hi = 0.0;
			for (auto& s : m_series)
			{
				if (!s.isValid() || !s.visible)
					continue;
				const stdvec<f64>& data = forXAxis ? s.xdata : s.ydata;
				for (f64 v : data)
				{
					if (!any) { lo = hi = v; any = true; }
					else { lo = std::min(lo, v); hi = std::max(hi, v); }
				}
			}
			if (!any)
			{
				r.min = 0.0;
				r.max = 1.0;
				return r;
			}
			if (lo == hi)
			{
				f64 pad = (lo != 0.0) ? std::abs(lo) * 0.1 : 1.0;
				lo -= pad;
				hi += pad;
			}
			r.min = lo;
			r.max = hi;
			return r;
		}

		stdvec<LineGraph::AxisTick> LineGraph::generate_nice_ticks(f64 minValue, f64 maxValue) const
		{
			stdvec<AxisTick> ticks;
			if (maxValue <= minValue)
				maxValue = minValue + 1.0;

			const f64 range = maxValue - minValue;
			const f64 roughStep = range / (f64)TargetTickCount;
			const f64 magnitude = std::pow(10.0, std::floor(std::log10(roughStep)));
			const f64 residual = roughStep / magnitude;
			f64 niceResidual;
			if (residual < 1.5)      niceResidual = 1.0;
			else if (residual < 3.0) niceResidual = 2.0;
			else if (residual < 7.0) niceResidual = 5.0;
			else                     niceResidual = 10.0;
			const f64 step = niceResidual * magnitude;
			if (step <= 0.0 || !std::isfinite(step))
				return ticks;

			const f64 niceMin = std::floor(minValue / step) * step;
			const f64 niceMax = std::ceil(maxValue / step) * step;

			// Decimal places needed to show a sub-1 step distinctly (e.g. step 0.25 -> 2 decimals).
			u8 precision = 0;
			if (step < 1.0)
			{
				f64 s = step;
				while (s < 1.0 && precision < 6) { s *= 10.0; precision++; }
			}

			for (f64 v = niceMin; v <= niceMax + (step * 0.5); v += step)
			{
				f64 cleanV = (std::abs(v) < step * 1e-6) ? 0.0 : v;  // clean up float noise around zero
				AxisTick t;
				t.value = cleanV;
				t.label = String("").add(cleanV, precision);
				ticks.push_back(t);
			}
			return ticks;
		}

		void LineGraph::ensure_axis_ready(void) const
		{
			if (m_xAxisDirty)
			{
				if (m_xTicks.empty())
				{
					AxisRange r = compute_data_range(true);
					m_autoXTicks = generate_nice_ticks(r.min, r.max);
					m_xRange = m_autoXTicks.empty() ? r : AxisRange { m_autoXTicks.front().value, m_autoXTicks.back().value };
				}
				else
				{
					f64 lo = m_xTicks.front().value, hi = m_xTicks.front().value;
					for (auto& t : m_xTicks) { lo = std::min(lo, t.value); hi = std::max(hi, t.value); }
					if (lo == hi) hi = lo + 1.0;
					m_xRange = { lo, hi };
				}
				m_xAxisDirty = false;
			}
			if (m_yAxisDirty)
			{
				if (m_yTicks.empty())
				{
					AxisRange r = compute_data_range(false);
					m_autoYTicks = generate_nice_ticks(r.min, r.max);
					m_yRange = m_autoYTicks.empty() ? r : AxisRange { m_autoYTicks.front().value, m_autoYTicks.back().value };
				}
				else
				{
					f64 lo = m_yTicks.front().value, hi = m_yTicks.front().value;
					for (auto& t : m_yTicks) { lo = std::min(lo, t.value); hi = std::max(hi, t.value); }
					if (lo == hi) hi = lo + 1.0;
					m_yRange = { lo, hi };
				}
				m_yAxisDirty = false;
			}
		}

		const stdvec<LineGraph::AxisTick>& LineGraph::active_x_ticks(void) const
		{
			ensure_axis_ready();
			return m_xTicks.empty() ? m_autoXTicks : m_xTicks;
		}

		const stdvec<LineGraph::AxisTick>& LineGraph::active_y_ticks(void) const
		{
			ensure_axis_ready();
			return m_yTicks.empty() ? m_autoYTicks : m_yTicks;
		}

		Rectangle LineGraph::compute_plot_area(ogfx::BasicRenderer2D& gfx) const
		{
			ensure_axis_ready();
			const auto& yTicks = active_y_ticks();

			f32 maxYLabelWidth = 0.0f;
			for (auto& t : yTicks)
				maxYLabelWidth = std::max(maxYLabelWidth, gfx.getStringDimensions(t.label, getFontSize()).x);

			const f32 tickTextHeight = (f32)getFontSize();
			const f32 pad = 6.0f;

			f32 left = pad + maxYLabelWidth + pad + 4.0f;
			if (!m_yAxisTitle.empty())
				left += tickTextHeight + pad;

			f32 bottom = pad + tickTextHeight + 4.0f + getPadding().bottom() + getPadding().top();
			if (!m_xAxisTitle.empty())
				bottom += tickTextHeight + pad;

			const f32 top = pad + getPadding().top() + 10.0f;
			const f32 right = pad + 4.0f + getPadding().right() + getPadding().left();

			return { left, top, std::max(1.0f, getw() - left - right), std::max(1.0f, geth() - top - bottom) };
		}

		f32 LineGraph::value_to_x(f64 value) const
		{
			const f64 range = m_xRange.max - m_xRange.min;
			const f64 t = (range != 0.0) ? (value - m_xRange.min) / range : 0.5;
			return m_plotArea.x + (f32)t * m_plotArea.w;
		}

		f32 LineGraph::value_to_y(f64 value) const
		{
			const f64 range = m_yRange.max - m_yRange.min;
			const f64 t = (range != 0.0) ? (value - m_yRange.min) / range : 0.5;
			return m_plotArea.y + m_plotArea.h - (f32)t * m_plotArea.h;
		}

		void LineGraph::onDraw(ogfx::BasicRenderer2D& gfx)
		{
			Rectangle local = compute_plot_area(gfx);
			const auto gpos = getGlobalContentPosition();
			m_plotArea = { gpos.x + local.x, gpos.y + local.y, local.w, local.h };

			draw_grid_and_axes(gfx);

			// The plot area is a sub-region of the widget (it excludes the axis-label margins), so
			// this clip is doing real work - unlike a clip against the widget's own full bounds,
			// which WidgetManager already applies for free when drawing this widget as a child.
			gfx.pushClippingRect(m_plotArea, true);
			// gfx.pushClippingRect(m_plotArea + /* Accounting for the border */ ostd::Rectangle { 1.0f, 1.0f, -2.0f, -2.0f }, true);
			draw_series(gfx);
			gfx.popClippingRect();

			draw_hover(gfx);
			draw_legend(gfx);
		}

		void LineGraph::draw_grid_and_axes(ogfx::BasicRenderer2D& gfx)
		{
			gfx.fillRect(m_plotArea, m_plotBgColor);

			// Sort working copies by actual screen position (not raw value) before thinning
			// overlapping labels. This matters for the Y axis in particular: ticks are generated
			// in ascending *value* order, but value_to_y() inverts the axis (a higher value sits
			// at a smaller pixel y), so walking them in value order actually walks the screen
			// bottom-to-top - the opposite of what the thinning check below assumes. Sorting by
			// screen position fixes that, and also makes thinning robust if setup() ticks weren't
			// supplied in sorted order to begin with.
			auto yTicks = active_y_ticks();
			std::sort(yTicks.begin(), yTicks.end(), [this](const AxisTick& a, const AxisTick& b) {
				return value_to_y(a.value) < value_to_y(b.value);
			});
			f32 lastLabelBottom = -1e9f;
			for (auto& t : yTicks)
			{
				const f32 y = value_to_y(t.value);
				const bool isZero = std::abs(t.value) < 1e-9;
				if (isGridEnabled())
					gfx.drawLine({ Vec2 { m_plotArea.x, y }, Vec2 { m_plotArea.x + m_plotArea.w, y } }, isZero ? m_axisLineColor : m_gridColor, isZero ? 2 : 1);

				Vec2 dims = gfx.getStringDimensions(t.label, getFontSize());
				f32 labelTop = y - dims.y * 0.5f;
				if (labelTop >= lastLabelBottom + MinLabelGap)
				{
					gfx.drawString(t.label, { m_plotArea.x - dims.x - 8.0f, labelTop }, m_axisLabelColor, getFontSize());
					lastLabelBottom = labelTop + dims.y;
				}
			}

			auto xTicks = active_x_ticks();
			std::sort(xTicks.begin(), xTicks.end(), [this](const AxisTick& a, const AxisTick& b) {
				return value_to_x(a.value) < value_to_x(b.value);
			});
			f32 lastLabelRight = -1e9f;
			for (auto& t : xTicks)
			{
				const f32 x = value_to_x(t.value);
				const bool isZero = std::abs(t.value) < 1e-9;
				if (isGridEnabled())
					gfx.drawLine({ Vec2 { x, m_plotArea.y }, Vec2 { x, m_plotArea.y + m_plotArea.h } }, isZero ? m_axisLineColor : m_gridColor, isZero ? 2 : 1);

				Vec2 dims = gfx.getStringDimensions(t.label, getFontSize());
				f32 labelLeft = x - dims.x * 0.5f;
				if (labelLeft >= lastLabelRight + MinLabelGap)
				{
					gfx.drawString(t.label, { labelLeft, m_plotArea.y + m_plotArea.h + 4.0f }, m_axisLabelColor, getFontSize());
					lastLabelRight = labelLeft + dims.x;
				}
			}

			gfx.drawRect(m_plotArea, m_axisLineColor, 1);

			if (!m_xAxisTitle.empty())
			{
				Rectangle titleBounds { m_plotArea.x, m_plotArea.y + m_plotArea.h + (f32)getFontSize() + 8.0f, m_plotArea.w, (f32)getFontSize() + 4.0f };
				gfx.drawCenteredString(m_xAxisTitle, titleBounds, m_axisTitleColor, getFontSize());
			}
			if (!m_yAxisTitle.empty())
			{
				const auto gpos = getGlobalContentPosition();
				gfx.drawString(m_yAxisTitle, { gpos.x, gpos.y }, m_axisTitleColor, getFontSize());
			}
		}

		void LineGraph::draw_series(ogfx::BasicRenderer2D& gfx)
		{
			for (auto& s : m_series)
			{
				if (!s.isValid() || !s.visible)
					continue;
				Vec2 prev { 0, 0 };
				bool hasPrev = false;
				for (u32 i = 0; i < s.xdata.size(); i++)
				{
					Vec2 p { value_to_x(s.xdata[i]), value_to_y(s.ydata[i]) };
					if (hasPrev)
						gfx.drawLine({ prev, p }, s.color, m_lineThickness);
					prev = p;
					hasPrev = true;
				}
				if (s.showPoints)
				{
					for (u32 i = 0; i < s.xdata.size(); i++)
						gfx.fillCircle(Vec2 { value_to_x(s.xdata[i]), value_to_y(s.ydata[i]) }, m_pointRadius, s.color);
				}
			}
		}

		void LineGraph::draw_legend(ogfx::BasicRenderer2D& gfx)
		{
			m_legendRowBounds.clear();
			if (!isLegendEnabled() || m_series.size() < 2)
				return;

			stdvec<String> labels;
			labels.reserve(m_series.size());
			for (u32 i = 0; i < m_series.size(); i++)
				labels.push_back(m_series[i].name.empty() ? String("Series ").add(i + 1) : m_series[i].name);

			const f32 pad = 6.0f;
			const f32 swatch = 10.0f;
			const f32 rowH = (f32)getFontSize() + 4.0f;

			f32 maxTextW = 0.0f;
			for (auto& label : labels)
				maxTextW = std::max(maxTextW, gfx.getStringDimensions(label, getFontSize()).x);

			const f32 boxW = pad * 3 + swatch + maxTextW;
			const f32 boxH = pad * 2 + rowH * (f32)m_series.size();

			// Index order matches eLegendPosition's declaration order (minus Auto), so a pinned
			// position can index straight into it.
			const Rectangle candidates[4] = {
				{ m_plotArea.x + pad,                           m_plotArea.y + pad, boxW, boxH },  // TopLeft
				{ m_plotArea.x + m_plotArea.w - boxW - pad,     m_plotArea.y + pad, boxW, boxH },  // TopRight
				{ m_plotArea.x + pad,                           m_plotArea.y + m_plotArea.h - boxH - pad, boxW, boxH },  // BottomLeft
				{ m_plotArea.x + m_plotArea.w - boxW - pad,     m_plotArea.y + m_plotArea.h - boxH - pad, boxW, boxH },  // BottomRight
			};
			if (m_legendPosition == eLegendPosition::Auto)
			{
				if (m_legendCornerDirty)
				{
					m_legendCornerIndex = pick_legend_corner(candidates);
					m_legendCornerDirty = false;
				}
			}
			else
				m_legendCornerIndex = (u32)m_legendPosition - 1;
			Rectangle box = candidates[m_legendCornerIndex];

			gfx.outlinedRect(box, m_legendBgColor, m_legendBorderColor, 1);

			f32 y = box.y + pad;
			for (u32 i = 0; i < m_series.size(); i++)
			{
				m_legendRowBounds.push_back({ box.x, y, box.w, rowH });

				const bool hovered = ((i32)i == m_legendHoverIndex);
				const bool visible = m_series[i].visible;
				const Color swatchColor = visible ? m_series[i].color : dim_color(m_series[i].color);
				const Color textColor = hovered ? m_legendHoverTextColor : (visible ? m_legendTextColor : dim_color(m_legendTextColor));

				Rectangle swatchRect { box.x + pad, y + (rowH - swatch) * 0.5f, swatch, swatch };
				gfx.fillRect(swatchRect, swatchColor);
				gfx.drawVCenteredString(labels[i], Rectangle { box.x + pad * 2 + swatch, y, maxTextW, rowH }, textColor, getFontSize());
				y += rowH;
			}
		}

		u32 LineGraph::pick_legend_corner(const Rectangle (&candidates)[4]) const
		{
			// Count plotted data points landing inside each candidate box; fewer points there
			// means the legend will sit over less of the graph. This is a point-based
			// approximation (a long segment between two sparse points could still pass through a
			// corner without either endpoint landing in it), not exact segment/rect intersection,
			// but it's cheap and matches the common case well.
			u32 counts[4] = { 0, 0, 0, 0 };
			for (auto& s : m_series)
			{
				if (!s.isValid() || !s.visible)
					continue;
				for (u32 i = 0; i < s.xdata.size(); i++)
				{
					Vec2 p { value_to_x(s.xdata[i]), value_to_y(s.ydata[i]) };
					for (u32 c = 0; c < 4; c++)
					{
						if (candidates[c].contains(p, true))
							counts[c]++;
					}
				}
			}

			u32 bestIdx = 1;  // TopRight - the historical fixed default, kept as the tie-break winner
			for (u32 c = 0; c < 4; c++)
			{
				if (counts[c] < counts[bestIdx])
					bestIdx = c;
			}
			return bestIdx;
		}

		bool LineGraph::find_nearest_point(const Vec2& pos, u32& outSeries, u32& outPoint) const
		{
			if (!m_plotArea.contains(pos, true))
				return false;

			f32 bestDistSq = HoverPickRadius * HoverPickRadius;
			bool found = false;
			for (u32 si = 0; si < m_series.size(); si++)
			{
				auto& s = m_series[si];
				if (!s.isValid() || !s.visible)
					continue;
				for (u32 pi = 0; pi < s.xdata.size(); pi++)
				{
					Vec2 p { value_to_x(s.xdata[pi]), value_to_y(s.ydata[pi]) };
					f32 dx = p.x - pos.x, dy = p.y - pos.y;
					f32 distSq = dx * dx + dy * dy;
					if (distSq < bestDistSq)
					{
						bestDistSq = distSq;
						outSeries = si;
						outPoint = pi;
						found = true;
					}
				}
			}
			return found;
		}

		void LineGraph::onMouseMoved(const Event& event)
		{
			const Vec2 pos { event.mouse->position_x, event.mouse->position_y };

			i32 legendRow = -1;
			for (u32 i = 0; i < m_legendRowBounds.size(); i++)
			{
				if (m_legendRowBounds[i].contains(pos, true))
				{
					legendRow = (i32)i;
					break;
				}
			}
			m_legendHoverIndex = legendRow;
			set_legend_cursor(legendRow >= 0);
			if (legendRow >= 0)
			{
				// Don't also show the nearest-data-point tooltip while over the legend overlay.
				m_hoverSeriesIndex = -1;
				m_hoverPointIndex = -1;
				return;
			}

			u32 si = 0, pi = 0;
			if (find_nearest_point(pos, si, pi))
			{
				m_hoverSeriesIndex = (i32)si;
				m_hoverPointIndex = (i32)pi;
			}
			else
			{
				m_hoverSeriesIndex = -1;
				m_hoverPointIndex = -1;
			}
		}

		void LineGraph::onMouseExited(const Event& event)
		{
			m_hoverSeriesIndex = -1;
			m_hoverPointIndex = -1;
			m_legendHoverIndex = -1;
			set_legend_cursor(false);
		}

		void LineGraph::onMousePressed(const Event& event)
		{
			if (event.mouse->button != ogfx::MouseEventData::eButton::Left)
				return;
			if (m_legendHoverIndex < 0 || (u32)m_legendHoverIndex >= m_series.size())
				return;

			Series& s = m_series[(u32)m_legendHoverIndex];
			s.visible = !s.visible;
			if (m_hoverSeriesIndex == m_legendHoverIndex)
			{
				m_hoverSeriesIndex = -1;
				m_hoverPointIndex = -1;
			}
			// Autoscaled axes exclude hidden series, so toggling one needs a range recompute.
			m_xAxisDirty = true;
			m_yAxisDirty = true;
			event.handle();
		}

		Vec2 LineGraph::draw_hover_box(ogfx::BasicRenderer2D& gfx, const Vec2& point, const Vec2& contentSize) const
		{
			const f32 pad = 6.0f;
			Rectangle box { point.x + 10.0f, point.y - contentSize.y - pad * 2.0f - 4.0f, contentSize.x + pad * 2.0f, contentSize.y + pad * 2.0f };
			if (box.x + box.w > m_plotArea.x + m_plotArea.w)
				box.x = point.x - box.w - 10.0f;
			if (box.y < m_plotArea.y)
				box.y = point.y + 10.0f;

			gfx.outlinedRect(box, m_hoverBgColor, m_crosshairColor, 1);
			return { box.x + pad, box.y + pad };
		}

		void LineGraph::draw_hover(ogfx::BasicRenderer2D& gfx)
		{
			if (m_hoverSeriesIndex < 0 || m_hoverPointIndex < 0 || (u32)m_hoverSeriesIndex >= m_series.size())
				return;
			auto& s = m_series[(u32)m_hoverSeriesIndex];
			if ((u32)m_hoverPointIndex >= s.xdata.size())
				return;

			const f64 xv = s.xdata[(u32)m_hoverPointIndex];
			const f64 yv = s.ydata[(u32)m_hoverPointIndex];
			const Vec2 p { value_to_x(xv), value_to_y(yv) };

			gfx.drawLine({ Vec2 { p.x, m_plotArea.y }, Vec2 { p.x, m_plotArea.y + m_plotArea.h } }, m_crosshairColor, 1);
			gfx.outlinedCircle(p, m_pointRadius + 2.0f, s.color, m_hoverCircleColor, 1);

			if (callback_onHover)
			{
				ostd::Object* ud = ((u32)m_hoverPointIndex < s.userData.size()) ? s.userData[(u32)m_hoverPointIndex] : nullptr;
				const ostd::Object& udRef = ud ? *ud : ostd::Object::Invalid();

				// The callback measures (via outSize) and draws in the same call, but the box has to
				// be drawn *behind* the content, so its size must be known first. Run the callback
				// once behind a zero-area clip - whatever it draws during this call is invisible,
				// regardless of where it tries to draw - purely to read back outSize.
				Vec2 outSize { 0, 0 };
				gfx.pushClippingRect({ 0, 0, 0, 0 }, true);
				bool handled = callback_onHover(gfx, xv, yv, udRef, Vec2 { 0, 0 }, outSize);
				gfx.popClippingRect();

				if (handled)
				{
					Vec2 contentTopLeft = draw_hover_box(gfx, p, outSize);
					Vec2 unused { 0, 0 };
					callback_onHover(gfx, xv, yv, udRef, contentTopLeft, unused);
					return;
				}
			}

			String text = "";
			if (!s.name.empty())
				text.add(s.name).add(": ");
			text.add("(").add(xv, 2).add(", ").add(yv, 2).add(")");

			Vec2 dims = gfx.getStringDimensions(text, getFontSize());
			Vec2 contentTopLeft = draw_hover_box(gfx, p, dims);
			gfx.drawString(text, contentTopLeft, m_hoverTextColor, getFontSize());
		}
	}
}
