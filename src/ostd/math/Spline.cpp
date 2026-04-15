#include "Spline.hpp"

namespace ostd
{
	Spline::Spline(void)
	{
	}

	void Spline::handleSignal(Signal& signal)
	{
		// if (!isEditable()) return;
		// if (isUsingCustomEventHandler() && m_eventHandlerCallback)
		// {
		// 	m_eventHandlerCallback(signal);
		// 	return;
		// }
		// if (signal.ID == BuiltinSignals::MousePressed)
		// {
		// 	ogfx::MouseEventData& evt = cast<ogfx::MouseEventData&>(signal.userData);
		// 	for (auto& node : m_points)
		// 	{
		// 		if (ostd::Rectangle(node.position.x - 20, node.position.y - 20, 40, 40).contains((f32)evt.position_x, (f32)evt.position_y))
		// 		{
		// 			m_selectedNode = &node;
		// 			return;
		// 		}
		// 	}
		// }
		// else if (signal.ID == BuiltinSignals::MouseReleased)
		// {
		// 	if (m_selectedNode != nullptr)
		// 		updateTotalLength();
		// 	m_selectedNode = nullptr;
		// }
		// else if (signal.ID == BuiltinSignals::MouseMoved)
		// {
		// 	ogfx::MouseEventData& evt = cast<ogfx::MouseEventData&>(signal.userData);
		// 	if (m_selectedNode != nullptr)
		// 		m_selectedNode->position = { (f32)evt.position_x, (f32)evt.position_y };
		// }
	}

	tSplineNode Spline::getPoint(f32 t)
	{
		i32 p0, p1, p2, p3;
		if (!isLooping())
		{
			p1 = (i32)t + 1;
			p2 = p1 + 1;
			p3 = p2 + 1;
			p0 = p1 - 1;
		}
		else
		{
			p1 = (i32)t;
			p2 = (p1 + 1) % m_points.size();
			p3 = (p2 + 1) % m_points.size();
			p0 = p1 >= 1 ? p1 - 1 : m_points.size() - 1;
		}

		t = t - (i32)t;

		f32 tt = t * t;
		f32 ttt = tt * t;

		f32 q1 = -ttt + 2.0f*tt - t;
		f32 q2 = 3.0f*ttt - 5.0f*tt + 2.0f;
		f32 q3 = -3.0f*ttt + 4.0f*tt + t;
		f32 q4 = ttt - tt;

		f32 tx = 0.5f * (m_points[p0].position.x * q1 + m_points[p1].position.x * q2 + m_points[p2].position.x * q3 + m_points[p3].position.x * q4);
		f32 ty = 0.5f * (m_points[p0].position.y * q1 + m_points[p1].position.y * q2 + m_points[p2].position.y * q3 + m_points[p3].position.y * q4);

		return { { tx, ty }, 0.0f };
	}

	tSplineNode Spline::getGradient(f32 t)
	{
		i32 p0, p1, p2, p3;
		if (!isLooping())
		{
			p1 = (i32)t + 1;
			p2 = p1 + 1;
			p3 = p2 + 1;
			p0 = p1 - 1;
		}
		else
		{
			p1 = (i32)t;
			p2 = (p1 + 1) % m_points.size();
			p3 = (p2 + 1) % m_points.size();
			p0 = p1 >= 1 ? p1 - 1 : m_points.size() - 1;
		}

		t = t - (i32)t;

		f32 tt = t * t;
		f32 ttt = tt * t;

		f32 q1 = -3.0f * tt + 4.0f*t - 1;
		f32 q2 = 9.0f*tt - 10.0f*t;
		f32 q3 = -9.0f*tt + 8.0f*t + 1.0f;
		f32 q4 = 3.0f*tt - 2.0f*t;

		f32 tx = 0.5f * (m_points[p0].position.x * q1 + m_points[p1].position.x * q2 + m_points[p2].position.x * q3 + m_points[p3].position.x * q4);
		f32 ty = 0.5f * (m_points[p0].position.y * q1 + m_points[p1].position.y * q2 + m_points[p2].position.y * q3 + m_points[p3].position.y * q4);

		return { { tx, ty }, 0.0f };
	}

	f32 Spline::updateSegmentLength(i32 node)
	{
		f32 fLength = 0.0f;
		f32 fStepSize = 0.005;

		tSplineNode old_point, new_point;
		old_point = getPoint((f32)node);

		for (f32 t = 0; t < 1.0f; t += fStepSize)
		{
			new_point = getGradient((f32)node + t);
			fLength += std::sqrt((new_point.position.x - old_point.position.x)*(new_point.position.x - old_point.position.x)
							+ (new_point.position.y - old_point.position.y)*(new_point.position.y - old_point.position.y));
			old_point = new_point;
		}
		return fLength;
	}

	f32 Spline::getNormalisedOffset(f32 p)
	{
		i32 i = 0;
		while (p > m_points[i].length)
		{
			p -= m_points[i].length;
			i++;
		}
		return (f32)i + (p / m_points[i].length);
	}

	void Spline::connectSignals(void)
	{
		if (m_signalsConnected) return;
		connectSignal(BuiltinSignals::MouseMoved);
		connectSignal(BuiltinSignals::MousePressed);
		connectSignal(BuiltinSignals::MouseReleased);
		m_signalsConnected = true;
	}

	void Spline::updateTotalLength(void)
	{
		m_totalLength = 0.0f;
		for (i32 i = 0; i < m_points.size(); i++)
		{
			m_totalLength += (m_points[i].length = updateSegmentLength(i));
		}
	}
}
