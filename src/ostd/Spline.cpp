#include "Spline.hpp"
#include "WindowBase.hpp"

namespace ostd
{
	Spline::Spline(void)
	{
	}

	void Spline::handleSignal(tSignal& signal)
	{
		if (!isEditable()) return;
		if (isUsingCustomEventHandler() && m_eventHandlerCallback)
		{
			m_eventHandlerCallback(signal);
			return;
		}
		if (signal.ID == tBuiltinSignals::MousePressed)
		{
			ogfx::MouseEventData& evt = static_cast<ogfx::MouseEventData&>(signal.userData);
			for (auto& node : m_points)
			{
				if (ostd::Rectangle(node.position.x - 20, node.position.y - 20, 40, 40).contains((float)evt.position_x, (float)evt.position_y))
				{
					m_selectedNode = &node;
					return;
				}
			}
		}
		else if (signal.ID == tBuiltinSignals::MouseReleased)
		{
			if (m_selectedNode != nullptr)
				updateTotalLength();
			m_selectedNode = nullptr;
		}
		else if (signal.ID == tBuiltinSignals::MouseMoved)
		{
			ogfx::MouseEventData& evt = static_cast<ogfx::MouseEventData&>(signal.userData);
			if (m_selectedNode != nullptr)
				m_selectedNode->position = { (float)evt.position_x, (float)evt.position_y };
		}
	}

	tSplineNode Spline::getPoint(float t)
	{
		int32_t p0, p1, p2, p3;
		if (!isLooping())
		{
			p1 = (int32_t)t + 1;
			p2 = p1 + 1;
			p3 = p2 + 1;
			p0 = p1 - 1;
		}
		else
		{
			p1 = (int32_t)t;
			p2 = (p1 + 1) % m_points.size();
			p3 = (p2 + 1) % m_points.size();
			p0 = p1 >= 1 ? p1 - 1 : m_points.size() - 1;
		}

		t = t - (int32_t)t;

		float tt = t * t;
		float ttt = tt * t;

		float q1 = -ttt + 2.0f*tt - t;
		float q2 = 3.0f*ttt - 5.0f*tt + 2.0f;
		float q3 = -3.0f*ttt + 4.0f*tt + t;
		float q4 = ttt - tt;

		float tx = 0.5f * (m_points[p0].position.x * q1 + m_points[p1].position.x * q2 + m_points[p2].position.x * q3 + m_points[p3].position.x * q4);
		float ty = 0.5f * (m_points[p0].position.y * q1 + m_points[p1].position.y * q2 + m_points[p2].position.y * q3 + m_points[p3].position.y * q4);

		return { { tx, ty }, 0.0f };
	}

	tSplineNode Spline::getGradient(float t)
	{
		int32_t p0, p1, p2, p3;
		if (!isLooping())
		{
			p1 = (int32_t)t + 1;
			p2 = p1 + 1;
			p3 = p2 + 1;
			p0 = p1 - 1;
		}
		else
		{
			p1 = (int32_t)t;
			p2 = (p1 + 1) % m_points.size();
			p3 = (p2 + 1) % m_points.size();
			p0 = p1 >= 1 ? p1 - 1 : m_points.size() - 1;
		}

		t = t - (int32_t)t;

		float tt = t * t;
		float ttt = tt * t;

		float q1 = -3.0f * tt + 4.0f*t - 1;
		float q2 = 9.0f*tt - 10.0f*t;
		float q3 = -9.0f*tt + 8.0f*t + 1.0f;
		float q4 = 3.0f*tt - 2.0f*t;

		float tx = 0.5f * (m_points[p0].position.x * q1 + m_points[p1].position.x * q2 + m_points[p2].position.x * q3 + m_points[p3].position.x * q4);
		float ty = 0.5f * (m_points[p0].position.y * q1 + m_points[p1].position.y * q2 + m_points[p2].position.y * q3 + m_points[p3].position.y * q4);

		return { { tx, ty }, 0.0f };
	}

	float Spline::updateSegmentLength(int32_t node)
	{
		float fLength = 0.0f;
		float fStepSize = 0.005;

		tSplineNode old_point, new_point;
		old_point = getPoint((float)node);

		for (float t = 0; t < 1.0f; t += fStepSize)
		{
			new_point = getGradient((float)node + t);
			fLength += std::sqrt((new_point.position.x - old_point.position.x)*(new_point.position.x - old_point.position.x)
							+ (new_point.position.y - old_point.position.y)*(new_point.position.y - old_point.position.y));
			old_point = new_point;
		}
		return fLength;
	}

	float Spline::getNormalisedOffset(float p)
	{
		int32_t i = 0;
		while (p > m_points[i].length)
		{
			p -= m_points[i].length;
			i++;
		}
		return (float)i + (p / m_points[i].length);
	}

	void Spline::connectSignals(void)
	{
		if (m_signalsConnected) return;
		connectSignal(tBuiltinSignals::MouseMoved);
		connectSignal(tBuiltinSignals::MousePressed);
		connectSignal(tBuiltinSignals::MouseReleased);
		m_signalsConnected = true;
	}

	void Spline::updateTotalLength(void)
	{
		m_totalLength = 0.0f;
		for (int i = 0; i < m_points.size(); i++)
		{
			m_totalLength += (m_points[i].length = updateSegmentLength(i));
		}
	}
}
