#ifndef __QUAD_TREE_HPP__
#define __QUAD_TREE_HPP__

#include <ostd/BaseObject.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Defines.hpp>
#include <vector>
#include <memory>

namespace ostd
{
	class QuadTree : public BaseObject
	{
		public: struct tElement
		{
			Vec2 pos;
			void* data;
			tElement(void);
			tElement(Vec2 position, void* userData);
		};
		public:
			inline QuadTree(void) : m_points(nullptr), m_sw(nullptr), m_nw(nullptr),
									m_se(nullptr),m_ne(nullptr), m_subdivided(false),
									m_capacity(0), m_currentSize(0)
									{ invalidate(); }
			inline QuadTree(Rectangle bounds, uint16_t capacity) { create(bounds, capacity); }
			inline ~QuadTree(void) { destroy(); }
			void destroy(void);
			void create(Rectangle bounds, uint16_t capacity);
			void subdivide(void);
			bool insert(Vec2 point, void* data = nullptr);
			void query(Rectangle range, std::vector<tElement*>& list);

		private:
			tElement* m_points;
			uint16_t m_currentSize;
			uint16_t m_capacity;
			Rectangle m_bounds;
			bool m_subdivided;
			un_ptr(QuadTree) m_nw;
			un_ptr(QuadTree) m_sw;
			un_ptr(QuadTree) m_ne;
			un_ptr(QuadTree) m_se;
	};
} 

#endif
