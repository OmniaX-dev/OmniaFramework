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

#include <vector>
#include <memory>
#include <ostd/data/BaseObject.hpp>
#include <ostd/math/Geometry.hpp>

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
			std::unique_ptr<QuadTree> m_nw;
			std::unique_ptr<QuadTree> m_sw;
			std::unique_ptr<QuadTree> m_ne;
			std::unique_ptr<QuadTree> m_se;
	};
}
