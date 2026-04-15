#include "PathFinder.hpp"
#include <limits>
#include <cmath>

namespace ostd
{
	KeyNode::KeyNode(i32 xx, i32 yy, f32 f)
	{
		x = xx;
		y = yy;
		fscore = f;
	}

	bool KeyNode::operator==(const KeyNode &other) const
	{
		return (x == other.x && y == other.y);
	}

	bool KeyNode::operator<(const KeyNode &other) const
	{
		return fscore < other.fscore;
	}

	std::size_t KeyNode::hashFunc::operator()(const KeyNode& k) const
	{
		return ((k.x + k.y) * (k.x + k.y + 1) / 2) + k.y;
	}



	OpenSet::OpenSet(void)
	{
		m_lowest = nullptr;
		m_lowest_index = -1;
	}

	bool OpenSet::isEmpty(void) const
	{
		return m_lowest_index == -1 || m_lowest == nullptr || m_list.empty();
	}

	void OpenSet::push(const KeyNode& k)
	{
		m_list.push_back(k);
		if (isEmpty() || k.fscore < m_lowest->fscore)
		{
			m_lowest = &(m_list[m_list.size() - 1]);
			m_lowest_index = m_list.size() - 1;
		}
	}

	bool OpenSet::contains(const KeyNode& k)
	{
		for (auto& kk : m_list)
			if (k == kk) return true;
		return false;
	}

	const KeyNode& OpenSet::operator[](u32 index) const
	{
		return m_list[index];
	}

	i32 OpenSet::size(void) const
	{
		return m_list.size();
	}

	void OpenSet::removeLowest(void)
	{
		remove(m_lowest_index);
	}

	void OpenSet::remove(u32 index)
	{
		if (isEmpty() || index >= size()) return;
		if (size() == 1)
		{
			clear();
			return;
		}
		m_list.erase(m_list.begin() + index);
		if (index == m_lowest_index)
		{
			m_lowest = &m_list[0];
			m_lowest_index = 0;
			for (u32 i = 1; i < m_list.size(); i++)
			{
				auto& k = m_list[i];
				if (k.fscore < m_lowest->fscore)
				{
					m_lowest = &k;
					m_lowest_index = i;
				}
			}
		}
	}

	void OpenSet::clear(void)
	{
		m_list.clear();
		m_lowest = nullptr;
		m_lowest_index = -1;
	}

	KeyNode& OpenSet::getLowest(void)
	{
		return *m_lowest;
	}

	i32 OpenSet::getLowestIndex(void)
	{
		return m_lowest_index;
	}



	PathFinder::PathFinder(void)
	{
		init(32, 32);
	}

	void PathFinder::init(i32 w, i32 h, eHeuristicType ht)
	{
		m_initialized = true;
		openSet.clear();
		gscores.clear();
		obstacle.clear();
		pathMap.clear();
		__ht = ht;
		m_width = w;
		m_height = h;
		for (i32 y = 0; y < h; y++)
		{
			for (i32 x = 0; x < w; x++)
			{
				gscores[{x, y, 0}] = std::numeric_limits<f32>::infinity();
			}
		}
	}

	void PathFinder::setObstacle(i32 x, i32 y, bool obst)
	{
		obstacle[{x, y, 0}] = obst;
	}

	void PathFinder::setKeyPoints(i32 startx, i32 starty, i32 endx, i32 endy)
	{
		if (!m_initialized) return;
		init(m_width, m_height, __ht);
		__goal = {endx, endy, 0.0f};
		__start = {startx, starty, 0.0f};
		__start.fscore = heuristics(__start);
		gscores[__start] = 0;
		openSet.push(__start);
	}

	std::deque<KeyNode> PathFinder::findPath(void)
	{
		if (!m_initialized) return std::deque<KeyNode>();
		KeyNode current;
		while (!openSet.isEmpty())
		{
			current = openSet.getLowest();
			if (current == __goal)
			{
				std::deque<KeyNode> path;
				while (pathMap.count(current) > 0)
				{
					current = pathMap[current];
					path.push_front(current);
				}
				return path;
			}
			openSet.removeLowest();
			i32 tmpx = current.x;
			i32 tmpy = current.y;
			f32 tg = 0;
			KeyNode neighbor;
			for (i8 x = -1; x <= 1; x++)
			{
				for (i8 y = -1; y <= 1; y++)
				{
					if (x == 0 && y == 0) continue;
					if (tmpx + x < 0 || tmpx + x >= m_width || tmpy + y < 0 || tmpy + y >= m_height) continue;
					neighbor.x = tmpx + x;
					neighbor.y = tmpy + y;
					if (obstacle[neighbor]) continue;
					tg = gscores[current] + ((tmpx == neighbor.x || tmpy == neighbor.y ? 10 : 14));
					if (tg < gscores[neighbor])
					{
						pathMap[neighbor] = current;
						gscores[neighbor] = tg;
						neighbor.fscore = tg + heuristics(neighbor);
						if (!openSet.contains(neighbor))
							openSet.push(neighbor);
					}
				}
			}
		}
		return std::deque<KeyNode>();
	}

	f32 PathFinder::heuristics(KeyNode n)
	{
		f32 hh = 0;
		switch (__ht)
		{
			case eHeuristicType::Euclidean:
				hh = (f32)std::sqrt(std::pow(n.x - __goal.x, 2) + std::pow(n.y - __goal.y, 2));
				break;
			case eHeuristicType::Manhattan:
				hh = std::abs(n.x - __goal.x) + std::abs(n.y - __goal.y);
				break;
			case eHeuristicType::Diagonal:
				hh = std::max(std::abs(n.x - __goal.x), std::abs(n.y - __goal.y));
				break;
			default:
				hh = 0.0f;
				break;
		}
		return hh;
	}
}
