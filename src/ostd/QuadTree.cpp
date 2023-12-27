#include "QuadTree.hpp"

namespace ostd
{
	QuadTree::tElement::tElement(void)
	{
		pos = { 0, 0 };
		data = nullptr;
	}

	QuadTree::tElement::tElement(Vec2 position, void* userData)
	{
		pos = position;
		data = userData;
	}
		
		
		
	void QuadTree::destroy(void)
	{
		m_nw = nullptr;
		m_sw = nullptr;
		m_ne = nullptr;
		m_se = nullptr;
		m_currentSize = 0;
		m_subdivided = false;
		if (m_points != nullptr)
			delete[] m_points;
		m_capacity = 0;
		m_bounds = { 0, 0, 0, 0 };
		setValid(false);
	}
	
	void QuadTree::create(Rectangle bounds, uint16_t capacity)
	{
		m_bounds = bounds;
		m_capacity = capacity;
		m_points = new tElement[capacity];
		m_subdivided = false;
		m_currentSize = 0;
		m_nw = nullptr;
		m_sw = nullptr;
		m_ne = nullptr;
		m_se = nullptr;
		setTypeName("ox::QuadTree");
		setValid(true);
	}

	void QuadTree::subdivide(void)
	{
		if (m_subdivided) return;
		auto center = m_bounds.getCenter();
		m_nw = new_un(QuadTree)(Rectangle(
			m_bounds.x, m_bounds.y, m_bounds.w / 2, m_bounds.h / 2
		), m_capacity);
		m_ne = new_un(QuadTree)(Rectangle(
			center.x, m_bounds.y, m_bounds.w / 2, m_bounds.h / 2
		), m_capacity);
		m_sw = new_un(QuadTree)(Rectangle(
			m_bounds.x, center.y, m_bounds.w / 2, m_bounds.h / 2
		), m_capacity);
		m_se = new_un(QuadTree)(Rectangle(
			center.x, center.y, m_bounds.w / 2, m_bounds.h / 2
		), m_capacity);
		m_subdivided = true;
	}

	bool QuadTree::insert(Vec2 point, void* data)
	{
		if (!m_bounds.contains({ point.x, point.y }, true))
			return false;
		if (m_currentSize < m_capacity - 1)
		{
			m_points[m_currentSize].pos = point;
			m_points[m_currentSize].data = data;
			m_currentSize++;
			return true;
		}
		subdivide();
		if (m_nw->insert(point, data))
			return true;
		if (m_ne->insert(point, data))
			return true;
		if (m_sw->insert(point, data))
			return true;
		if (m_se->insert(point, data))
			return true;
		return false;
	}

	void QuadTree::query(Rectangle range, std::vector<tElement*>& list)
	{
		if (!m_bounds.intersects(range, true))
			return;
		for (uint16_t i = 0; i < m_currentSize; i++)
		{
			auto& elem = m_points[i];
			if (range.contains({ elem.pos.x, elem.pos.y }))
				list.push_back(&elem);
		}
		if (m_subdivided)
		{
			m_nw->query(range, list);
			m_ne->query(range, list);
			m_sw->query(range, list);
			m_se->query(range, list);
		}
	}
} 
