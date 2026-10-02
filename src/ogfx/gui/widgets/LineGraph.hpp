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

#include <ogfx/gui/widgets/Widget.hpp>
#include <ostd/data/BaseObject.hpp>
#include <utility>
#include <functional>

namespace ogfx
{
	namespace gui
	{
		// A line graph over two continuous (f64) axes. Each axis is a numeric line labeled at
		// arbitrary points by setup() (e.g. month-boundary labels over a daily-resolution axis);
		// plot() adds a data series on top of that scale. If an axis is never given explicit
		// ticks, it autoscales from the plotted data and generates its own "nice" tick values.
		class LineGraph : public Widget
		{
			public: enum class eLegendPosition : u8 { Auto = 0, TopLeft, TopRight, BottomLeft, BottomRight };

			public: struct AxisTick
			{
				String label { "" };
				f64    value { 0.0 };
			};

			public: struct Series
			{
				String      name       { "" };
				Color       color      { Colors::Transparent };  // resolved to a palette color by plot()
				bool        showPoints { true };
				bool        visible    { true };  // toggled by clicking this series's legend entry
				stdvec<f64> xdata;
				stdvec<f64> ydata;
				// Parallel to xdata/ydata (one entry per point) if non-empty; passed to the hover
				// callback. Not owned - LineGraph never deletes these.
				stdvec<ostd::BaseObject*> userData;

				inline bool isValid(void) const { return !xdata.empty() && xdata.size() == ydata.size(); }
			};

			private: struct AxisRange { f64 min { 0.0 }; f64 max { 1.0 }; };

			public:
				inline LineGraph(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				LineGraph& create(void);
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(ogfx::BasicRenderer2D& gfx) override;
				void onMouseMoved(const Event& event) override;
				void onMouseExited(const Event& event) override;
				void onMousePressed(const Event& event) override;

				// Ticks are {label, value} pairs; an axis's numeric range is [min value, max value]
				// among its own ticks. Pass an empty list for either axis to autoscale it instead.
				void setup(const stdvec<std::pair<String, f64>>& x_axis, const stdvec<std::pair<String, f64>>& y_axis);
				inline void setXAxisTitle(const String& title) { m_xAxisTitle = title; }
				inline void setYAxisTitle(const String& title) { m_yAxisTitle = title; }

				// Adds a new series on top of the current axis setup and returns its index.
				// Colors::Transparent (the default) auto-assigns the next palette color. userData, if
				// non-empty, must match xdata/ydata in size - one entry per point, passed to the hover
				// callback (see setHoverCallback()); pass {} (the default) to omit it.
				// Rejects (returns (u32)-1, logs a warning) if xdata/ydata sizes differ, or userData is
				// non-empty and doesn't match them.
				u32 plot(const stdvec<f64>& xdata, const stdvec<f64>& ydata, const String& name = "", const Color& color = Colors::Transparent, bool showPoints = true, const stdvec<ostd::BaseObject*>& userData = {});
				bool removeSeries(u32 index);
				bool removeSeries(const String& name);
				void clearSeries(void);
				inline u32 getSeriesCount(void) const { return (u32)m_series.size(); }
				Series& getSeries(u32 index);
				const Series& getSeries(u32 index) const;
				// Forces the autoscaled axis/ticks to recompute, for callers who mutated a Series's
				// xdata/ydata in place via getSeries() rather than through plot().
				inline void refreshAutoScale(void) { m_xAxisDirty = true; m_yAxisDirty = true; }

				// Runs instead of the default hover tooltip rendering when set. userData is the
				// hovered point's entry from its series (ostd::BaseObject::InvalidRef() if that entry
				// is null, or if the series has no userData at all). Return true to fully replace the
				// default rendering, or false to let it still run (e.g. to draw something additional
				// alongside it rather than instead of it).
				//
				// The widget still owns the tooltip's background box and its positioning (including
				// flipping to the other side of the point to stay inside the plot area) - topLeft is
				// where the callback should draw its content, and it must report the content's size
				// through outSize so the box can be sized to fit it. Because that size has to be known
				// before the box (and therefore topLeft) can be finalized, but is only known by actually
				// calling the callback, it is invoked twice per hovered frame: once "invisibly" (behind
				// a zero-area clip, so nothing it draws is seen) purely to read outSize, and once for
				// real at the final topLeft. Keep it a pure function of (x, y, userData) - no side
				// effects - since it won't always visibly run when called.
				using HoverCallback = std::function<bool(ogfx::BasicRenderer2D& gfx, f64 xdata, f64 ydata, ostd::BaseObject& userData, const Vec2& topLeft, Vec2& outSize)>;
				inline void setHoverCallback(HoverCallback callback) { callback_onHover = std::move(callback); }

				OSTD_PARAM_GETSET(Color, PlotBackgroundColor, m_plotBgColor);
				OSTD_PARAM_GETSET(Color, GridColor, m_gridColor);
				OSTD_BOOL_PARAM_GETSET_E(Grid, m_showGrid);
				OSTD_PARAM_GETSET(Color, AxisLineColor, m_axisLineColor);
				OSTD_PARAM_GETSET(Color, AxisLabelColor, m_axisLabelColor);
				OSTD_PARAM_GETSET(Color, AxisTitleColor, m_axisTitleColor);
				OSTD_PARAM_GETSET(f32, PointRadius, m_pointRadius);
				OSTD_PARAM_GETSET(i32, LineThickness, m_lineThickness);
				OSTD_BOOL_PARAM_GETSET_E(Legend, m_showLegend);
				// Auto (the default) places the legend in whichever corner currently holds the
				// fewest plotted data points, re-evaluated every frame; pin a specific corner instead
				// if you'd rather it never moves.
				OSTD_PARAM_GETSET(eLegendPosition, LegendPosition, m_legendPosition);
				OSTD_PARAM_GETSET(Color, LegendBackgroundColor, m_legendBgColor);
				OSTD_PARAM_GETSET(Color, LegendTextColor, m_legendTextColor);
				OSTD_PARAM_GETSET(Color, LegendHoverTextColor, m_legendHoverTextColor);
				OSTD_PARAM_GETSET(Color, LegendBorderColor, m_legendBorderColor);
				OSTD_PARAM_GETSET(Color, CrosshairColor, m_crosshairColor);
				OSTD_PARAM_GETSET(Color, HoverLabelBackgroundColor, m_hoverBgColor);
				OSTD_PARAM_GETSET(Color, HoverLabelTextColor, m_hoverTextColor);
				OSTD_PARAM_GETSET(Color, HoverCircleColor, m_hoverCircleColor);

			private:
				static Color default_palette_color(u32 index);
				static Color dim_color(const Color& c);
				void set_legend_cursor(bool active);
				void ensure_axis_ready(void) const;
				AxisRange compute_data_range(bool forXAxis) const;
				stdvec<AxisTick> generate_nice_ticks(f64 minValue, f64 maxValue) const;
				const stdvec<AxisTick>& active_x_ticks(void) const;
				const stdvec<AxisTick>& active_y_ticks(void) const;
				Rectangle compute_plot_area(ogfx::BasicRenderer2D& gfx) const;
				f32 value_to_x(f64 value) const;
				f32 value_to_y(f64 value) const;
				void draw_grid_and_axes(ogfx::BasicRenderer2D& gfx);
				void draw_series(ogfx::BasicRenderer2D& gfx);
				void draw_legend(ogfx::BasicRenderer2D& gfx);
				u32 pick_legend_corner(const Rectangle (&candidates)[4]) const;
				void draw_hover(ogfx::BasicRenderer2D& gfx);
				// Positions and draws the tooltip's background box for the given content size
				// (flipping to the other side of point if it would overflow the plot area), and
				// returns where the content itself should be drawn.
				Vec2 draw_hover_box(ogfx::BasicRenderer2D& gfx, const Vec2& point, const Vec2& contentSize) const;
				bool find_nearest_point(const Vec2& localPos, u32& outSeries, u32& outPoint) const;

			private:
				stdvec<AxisTick> m_xTicks, m_yTicks;                        // explicit, from setup(); empty = autoscale
				mutable stdvec<AxisTick> m_autoXTicks, m_autoYTicks;        // generated when autoscaling
				mutable bool m_xAxisDirty { true };
				mutable bool m_yAxisDirty { true };
				mutable AxisRange m_xRange, m_yRange;                       // resolved range for the current frame
				String m_xAxisTitle { "" };
				String m_yAxisTitle { "" };

				stdvec<Series> m_series;
				Series m_invalidSeries;
				HoverCallback callback_onHover { nullptr };
				u32 m_nextPaletteIndex { 0 };

				mutable Rectangle m_plotArea { 0, 0, 0, 0 };                // cached by the last draw, reused for hit-testing
				mutable stdvec<Rectangle> m_legendRowBounds;                // cached by the last draw, one per series, for hit-testing

				i32 m_hoverSeriesIndex { -1 };
				i32 m_hoverPointIndex { -1 };
				i32 m_legendHoverIndex { -1 };
				bool m_legendCursorActive { false };
				// The auto-picked corner is sticky: resolved once when the series set structurally
				// changes (plot()/removeSeries()/clearSeries()), not re-evaluated on every draw or
				// on a visibility toggle - otherwise clicking an entry to hide it could shift where
				// the fewest points are and jump the legend out from under the cursor mid-click.
				u32 m_legendCornerIndex { 1 };  // TopRight
				bool m_legendCornerDirty { true };

				Color m_plotBgColor { 30, 30, 30 };
				Color m_gridColor { 60, 60, 60 };
				bool m_showGrid { true };
				Color m_axisLineColor { 120, 120, 120 };
				Color m_axisLabelColor { Colors::White };
				Color m_axisTitleColor { Colors::White };
				f32 m_pointRadius { 3.0f };
				i32 m_lineThickness { 2 };
				bool m_showLegend { true };
				eLegendPosition m_legendPosition { eLegendPosition::Auto };
				Color m_legendBgColor { 20, 20, 20, 220 };
				Color m_legendTextColor { Colors::White };
				Color m_legendHoverTextColor { 255, 230, 120 };
				Color m_legendBorderColor { 90, 90, 90 };
				Color m_crosshairColor { 150, 150, 150 };
				Color m_hoverBgColor { 20, 20, 20, 230 };
				Color m_hoverTextColor { Colors::White };
				Color m_hoverCircleColor { Colors::White };

				inline static constexpr f32 MinLabelGap = 8.0f;
				inline static constexpr f32 HoverPickRadius = 16.0f;
				inline static constexpr i32 TargetTickCount = 5;
		};
	}
}
