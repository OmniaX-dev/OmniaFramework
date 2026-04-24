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

#include <deque>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <ostd/data/Types.hpp>

namespace ostd
{
	// ============================================================================
	// StaticHashMap<K, V>
	// A drop-in replacement for stdumap (std::unordered_map) that preserves
	// insertion order. Provides:
	//   - O(1) lookup by key   (via internal unordered_map index)
	//   - O(1) lookup by index (via internal deque)
	//   - O(n) removal         (index rebuild after erase)
	//   - Insertion-order iteration via begin()/end()
	// ============================================================================
	template <typename K, typename V>
	class StaticHashMap
	{
	public:
		using value_type     = std::pair<K, V>;
		using storage_type   = std::deque<value_type>;
		using iterator       = typename storage_type::iterator;
		using const_iterator = typename storage_type::const_iterator;

	// ============================================================
	//  Capacity
	// ============================================================
	public:
		inline i32  size(void)  const { return cast<i32>(m_entries.size()); }
		inline bool empty(void) const { return m_entries.empty(); }

	// ============================================================
	//  Element access — key based
	// ============================================================
	public:
		// STL-style operator[]: inserts a default-constructed V if key absent,
		// then returns a reference. Allows  map["key"] = value  idiom.
		V& operator[](const K& key)
		{
			auto it = m_index.find(key);
			if (it == m_index.end())
			{
				m_index[key] = cast<i32>(m_entries.size());
				m_entries.push_back({ key, V{} });
				return m_entries.back().second;
			}
			return m_entries[it->second].second;
		}

		// Bounds-checked key access — throws std::out_of_range if absent.
		V& at(const K& key)
		{
			auto it = m_index.find(key);
			if (it == m_index.end())
				throw std::out_of_range("StaticHashMap::at — key not found");
			return m_entries[it->second].second;
		}

		const V& at(const K& key) const
		{
			auto it = m_index.find(key);
			if (it == m_index.end())
				throw std::out_of_range("StaticHashMap::at — key not found");
			return m_entries[it->second].second;
		}

	// ============================================================
	//  Element access — index based  (vector-style)
	// ============================================================
	public:
		// Direct index access — no bounds check (mirrors std::vector::operator[])
		V& operator[](i32 index)
		{
			return m_entries[cast<u32>(index)].second;
		}

		const V& operator[](i32 index) const
		{
			return m_entries[cast<u32>(index)].second;
		}

		// Bounds-checked index access — throws std::out_of_range
		V& at(i32 index)
		{
			if (index < 0 || index >= size())
				throw std::out_of_range("StaticHashMap::at — index out of range");
			return m_entries[cast<u32>(index)].second;
		}

		const V& at(i32 index) const
		{
			if (index < 0 || index >= size())
				throw std::out_of_range("StaticHashMap::at — index out of range");
			return m_entries[cast<u32>(index)].second;
		}

		// Access to the full pair at a given index (key + value)
		value_type& entry(i32 index)
		{
			return m_entries[cast<u32>(index)];
		}

		const value_type& entry(i32 index) const
		{
			return m_entries[cast<u32>(index)];
		}

		// Returns the key at a given insertion-order index
		const K& keyAt(i32 index) const
		{
			return m_entries[cast<u32>(index)].first;
		}

		V& front(void) { return m_entries.front().second; }
		V& back(void)  { return m_entries.back().second;  }
		const V& front(void) const { return m_entries.front().second; }
		const V& back(void)  const { return m_entries.back().second;  }

	// ============================================================
	//  Lookup
	// ============================================================
	public:
		// Returns 1 if key exists, 0 otherwise — mirrors stdumap::count()
		inline i32 count(const K& key) const
		{
			return m_index.count(key) ? 1 : 0;
		}

		// C++20-style contains — cleaner than count() for boolean checks
		inline bool contains(const K& key) const
		{
			return m_index.count(key) > 0;
		}

		// Returns the insertion-order index of a key, or -1 if not found
		inline i32 indexOf(const K& key) const
		{
			auto it = m_index.find(key);
			if (it == m_index.end()) return -1;
			return it->second;
		}

		// Returns an iterator to the entry with the given key, or end() if absent.
		// The iterator dereferences to std::pair<K, V>, same as stdumap.
		iterator find(const K& key)
		{
			auto it = m_index.find(key);
			if (it == m_index.end()) return m_entries.end();
			return m_entries.begin() + it->second;
		}

		const_iterator find(const K& key) const
		{
			auto it = m_index.find(key);
			if (it == m_index.end()) return m_entries.end();
			return m_entries.begin() + it->second;
		}

	// ============================================================
	//  Insertion
	// ============================================================
	public:
		// Appends at the end. If key already exists the value is updated in-place,
		// insertion order is preserved (no re-insertion at end).
		// Returns a reference to the stored value.
		V& insert(const K& key, const V& value)
		{
			auto it = m_index.find(key);
			if (it != m_index.end())
			{
				m_entries[it->second].second = value;
				return m_entries[it->second].second;
			}
			m_index[key] = cast<i32>(m_entries.size());
			m_entries.push_back({ key, value });
			return m_entries.back().second;
		}

		// STL-style pair insert — mirrors stdumap::insert({k, v})
		// Returns pair<iterator, bool>: iterator to element, true if inserted
		std::pair<iterator, bool> insert(const value_type& kv)
		{
			auto it = m_index.find(kv.first);
			if (it != m_index.end())
				return { m_entries.begin() + it->second, false };
			i32 idx = cast<i32>(m_entries.size());
			m_index[kv.first] = idx;
			m_entries.push_back(kv);
			return { m_entries.begin() + idx, true };
		}

		// Inserts at a specific position (O(n) — rebuilds indices from pos onward)
		// If key already exists, does nothing and returns false.
		bool insertAt(i32 pos, const K& key, const V& value)
		{
			if (contains(key)) return false;
			pos = std::clamp(pos, 0, size());
			m_entries.insert(m_entries.begin() + pos, { key, value });
			// Rebuild index for every entry from pos onward
			for (i32 i = pos; i < size(); i++)
				m_index[m_entries[cast<u32>(i)].first] = i;
			return true;
		}

		// In-place construction — mirrors stdumap::emplace()
		// Returns pair<iterator, bool>
		template <typename... Args>
		std::pair<iterator, bool> emplace(const K& key, Args&&... args)
		{
			auto it = m_index.find(key);
			if (it != m_index.end())
				return { m_entries.begin() + it->second, false };
			i32 idx = cast<i32>(m_entries.size());
			m_index[key] = idx;
			m_entries.emplace_back(key, V(std::forward<Args>(args)...));
			return { m_entries.begin() + idx, true };
		}

		// Appends a pair — mirrors push_back on a vector of pairs
		inline void push_back(const value_type& kv) { insert(kv); }

		// Removes the last element
		void pop_back(void)
		{
			if (m_entries.empty()) return;
			m_index.erase(m_entries.back().first);
			m_entries.pop_back();
		}

	// ============================================================
	//  Removal
	// ============================================================
	public:
		// Removes by key — O(n) index rebuild for entries after the removed slot.
		// Returns true if the key was found and removed.
		bool erase(const K& key)
		{
			auto it = m_index.find(key);
			if (it == m_index.end()) return false;
			i32 pos = it->second;
			m_entries.erase(m_entries.begin() + pos);
			m_index.erase(it);
			// Fix up indices for everything that shifted
			for (i32 i = pos; i < size(); i++)
				m_index[m_entries[cast<u32>(i)].first] = i;
			return true;
		}

		// Removes by insertion-order index — O(n) index rebuild.
		bool eraseAt(i32 index)
		{
			if (index < 0 || index >= size()) return false;
			return erase(m_entries[cast<u32>(index)].first);
		}

		// Removes all entries — O(1)
		void clear(void)
		{
			m_entries.clear();
			m_index.clear();
		}

	// ============================================================
	//  Iteration  (insertion order, mirrors stdumap range-for)
	// ============================================================
	public:
		inline iterator       begin(void)        { return m_entries.begin(); }
		inline iterator       end(void)          { return m_entries.end();   }
		inline const_iterator begin(void)  const { return m_entries.begin(); }
		inline const_iterator end(void)    const { return m_entries.end();   }
		inline const_iterator cbegin(void) const { return m_entries.cbegin(); }
		inline const_iterator cend(void)   const { return m_entries.cend();   }

	// ============================================================
	//  Private
	// ============================================================
	private:
		storage_type                         m_entries; // insertion-ordered pairs
		std::unordered_map<K, i32>           m_index;   // key → index into m_entries
	};

	// Convenience alias that matches the stdumap naming convention in Types.hpp
	template <typename K, typename V>
	using stdomap = StaticHashMap<K, V>;

} // namespace ostd
