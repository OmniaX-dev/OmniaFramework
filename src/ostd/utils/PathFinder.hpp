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

#include <unordered_map>
#include <deque>
#include <ostd/data/Types.hpp>
#include <ostd/utils/Defines.hpp>

namespace ostd
{
	enum class eHeuristicType
	{
		Euclidean = 0,
		Manhattan,
		Diagonal
	};

	struct KeyNode
	{
		i32 x;
		i32 y;
		f32 fscore;

		KeyNode(i32 xx = 0, i32 yy = 0, f32 f = 0.0f);
		bool operator==(const KeyNode &other) const;
		bool operator<(const KeyNode &other) const;

		struct hashFunc
		{
			std::size_t operator()(const KeyNode& k) const;
		};
	};

	struct OpenSet
	{
		OpenSet(void);
		bool isEmpty(void) const;
		void push(const KeyNode& k);
		bool contains(const KeyNode& k);
		const KeyNode& operator[](u32 index) const;
		i32 size(void) const;
		void removeLowest(void);
		void remove(u32 index);
		void clear(void);
		KeyNode& getLowest(void);
		i32 getLowestIndex(void);

		private:
			i32 m_lowest_index;
			stdvec<KeyNode> m_list;
			KeyNode* m_lowest;
	};

	class PathFinder
	{
		public:
			PathFinder(void);
			void init(i32 w, i32 h, eHeuristicType ht = eHeuristicType::Euclidean);
			void setObstacle(i32 x, i32 y, bool obst = true);
			void setKeyPoints(i32 startx, i32 starty, i32 endx, i32 endy);
			std::deque<KeyNode> findPath(void);

		private:
			f32 heuristics(KeyNode n);

		private:
			std::unordered_map<KeyNode, f32, KeyNode::hashFunc> gscores;
			std::unordered_map<KeyNode, KeyNode, KeyNode::hashFunc> pathMap;
			std::unordered_map<KeyNode, bool, KeyNode::hashFunc> obstacle;
			OpenSet openSet;

			KeyNode __start;
			KeyNode __goal;
			eHeuristicType __ht;

			i32 m_width;
			i32 m_height;

			bool m_initialized;
	};
}
