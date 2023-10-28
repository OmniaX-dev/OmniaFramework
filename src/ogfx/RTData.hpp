#ifndef __RTDATA_HPP__
#define __RTDATA_HPP__

#include <ostd/Types.hpp>
#include <ostd/Geometry.hpp>

namespace ogfx
{
	class RTData
	{
		public:
			inline static float DeltaTime { 0.0f };
			inline static ostd::IPoint windowSize { 0, 0 };
			// static uint32_t newCustomSignal(uint32_t sub_id);
	};
}

#endif