#pragma once

#include <ostd/data_types/Types.hpp>
#include <ostd/math/Geometry.hpp>
#include <ogfx/Image.hpp>

namespace ogfx
{
	class Animation
	{
		public: struct AnimationData {
				int32_t length;
				int32_t speed;
				int32_t still_frame;

				int32_t row_offset;
				int32_t column_offset;

				int32_t rows;
				int32_t columns;

				int32_t frame_width;
				int32_t frame_height;

				bool still;
				bool backwards;
				bool random;

				AnimationData(void)
				{
					length = 3;
					speed = 10;
					row_offset = 0;
					column_offset = 0;
					rows = 1;
					columns = 3;
					frame_width = 32;
					frame_height = 32;
					still = false;
					backwards = true;
					random = false;
					still_frame = 1;
				}
			};
	public:
		Animation(void);
		Animation(int32_t frames, int32_t columns, int32_t rows, int32_t frame_width, int32_t frame_height);
		Animation(AnimationData& ad);
		void create(int32_t frames, int32_t columns, int32_t rows, int32_t frame_width, int32_t frame_height);
		void create(AnimationData& ad);
		void update(void);
		void resetAnimation(void);

		inline void setFrameNumber(int32_t n) { m_frames = n; }
		inline void setColumnNumber(int32_t n) { m_columns = n; }
		inline void setBackwards(bool b) { m_backwards = b; }
		inline void setSpeed(int32_t d) { m_frame_time = d; }
		inline void setColumnOffset(int32_t o) { m_column_offset = o; }
		inline void setRowOffset(int32_t o) { m_row_offset = o; }
		inline void setStill(bool s) { m_still = s; }
		inline void setSpriteSheet(const Image& img) { m_spriteSheet = &img; }

		inline int32_t getFrameNumber(void) const { return m_frames; }
		inline int32_t getColumnNumber(void) const { return m_columns; }
		inline bool getBackwards(void) const { return m_backwards; }
		inline int32_t getDelay(void) const { return m_frame_time; }
		inline int32_t getColumnOffset(void) const { return m_column_offset; }
		inline int32_t getRowOffset(void) const { return m_row_offset; }
		inline bool isStill(void) const { return m_still; }
		inline ostd::Rectangle getFrameRect(void) const { return m_frame_rect; }
		inline const Image& getSpriteSheet(void) const { return (m_spriteSheet != nullptr ? *m_spriteSheet : InvalidImage); }
		inline bool hasImage(void) const { return m_spriteSheet != nullptr; }

	private:
		const Image* m_spriteSheet { nullptr };
		inline static const Image InvalidImage;

		int32_t m_frames;
		int32_t m_rows;
		int32_t m_columns;
		int32_t m_frame_width;
		int32_t m_frame_height;

		int32_t m_column_offset;
		int32_t m_row_offset;

		int32_t m_frame_time;
		int32_t m_current_time;
		int32_t m_current_frame;

		bool m_backwards;
		bool m_back;
		bool m_still;
		bool m_random;

		ostd::Rectangle m_frame_rect;
	};
}
