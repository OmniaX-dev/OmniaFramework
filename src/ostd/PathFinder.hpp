#ifndef __PATH_FINDER_HPP__
#define __PATH_FINDER_HPP__

#include <vector>
#include <unordered_map>
#include <deque>
#include <cstdint>

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
		int32_t x;
		int32_t y;
		float fscore;

		KeyNode(int32_t xx = 0, int32_t yy = 0, float f = 0.0f);
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
		const KeyNode& operator[](uint32_t index) const;
		int32_t size(void) const;
		void removeLowest(void);
		void remove(uint32_t index);
		void clear(void);
		KeyNode& getLowest(void);
		int32_t getLowestIndex(void);

		private:
			int32_t m_lowest_index;
			std::vector<KeyNode> m_list;
			KeyNode* m_lowest;
	};

	class PathFinder
	{
		public:
			PathFinder(void);
			void init(int32_t w, int32_t h, eHeuristicType ht = eHeuristicType::Euclidean);
			void setObstacle(int32_t x, int32_t y, bool obst = true);
			void setKeyPoints(int32_t startx, int32_t starty, int32_t endx, int32_t endy);
			std::deque<KeyNode> findPath(void);

		private:
			float heuristics(KeyNode n);

		private:
			std::unordered_map<KeyNode, float, KeyNode::hashFunc> gscores;
			std::unordered_map<KeyNode, KeyNode, KeyNode::hashFunc> pathMap;
			std::unordered_map<KeyNode, bool, KeyNode::hashFunc> obstacle;
			OpenSet openSet;

			KeyNode __start;
			KeyNode __goal;
			eHeuristicType __ht;

			int32_t m_width;
			int32_t m_height;

			bool m_initialized;
	};
} // namespace ox

#endif
