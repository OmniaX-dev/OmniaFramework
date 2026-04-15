/*
    OmniaFramework - A collection of useful functionality
    Copyright (C) 2025  OmniaX-Dev

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

#include <functional>
#include <ostd/math/Geometry.hpp>
#include <ostd/utils/Signals.hpp>

namespace ostd
{
	struct tSplineNode
	{
		Vec2 position;
		f32 length;
	};

	class Spline : public BaseObject
	{
		public:
			using EventHandlerCallback = std::function<void(Signal&)>;
			using DrawCallback = std::function<void(f32 resolution, f32 lineWidth, f32 controlPointSize)>;

		public:
			Spline(void);
			void handleSignal(Signal& signal) override;
			tSplineNode getPoint(f32 t);
			tSplineNode getGradient(f32 t);
			f32 updateSegmentLength(i32 node);
			f32 getNormalisedOffset(f32 p);
			void connectSignals(void);
			void updateTotalLength(void);
			inline void draw(f32 resolution = 0.01f, f32 lineWidth = 4.0f, f32 controlPointSize = 9.0f) { if (m_drawCallback) m_drawCallback(resolution, lineWidth, controlPointSize); }
			inline bool exists(void) { return m_points.size() > 1; }
			inline void addPoint(Vec2 position) { m_points.push_back({ position, 0.0f }); updateTotalLength(); }
			inline f32 getTotalLength(void) { return m_totalLength; }
			inline bool isEditable(void) { return m_editable; }
			inline bool isLooping(void) { return m_looping; }
			inline void setEditable(bool e) { m_editable = e; }
			inline void setLooping(bool l) { m_looping = l; }
			inline u32 getPointCount(void) { return m_points.size(); }
			inline void clear(void) { m_points.clear(); }
			inline bool isEnabled(void) { return m_enabled; }
			inline void enable(bool e = true) { m_enabled = e; }
			inline bool isUsingCustomEventHandler(void) { return m_useCustomEventHandler; }
			inline void useCustomEventHandler(bool use = true, EventHandlerCallback callback = nullptr);
			inline void setDrawCallback(DrawCallback drawCallback) { m_drawCallback = drawCallback; }

		protected:
			stdvec<tSplineNode> m_points;
			f32 m_totalLength = 0.0f;
			tSplineNode* m_selectedNode { nullptr };
			bool m_editable { false };
			bool m_looping { true };
			bool m_enabled { true };
			bool m_signalsConnected { false };
			bool m_useCustomEventHandler { false };
			EventHandlerCallback m_eventHandlerCallback { nullptr };
			DrawCallback m_drawCallback { nullptr };
	};
}
