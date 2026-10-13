#include <cstddef>

namespace osnd
{
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wc23-extensions"
	extern const unsigned char upright_piano_kw_sf2[] = {
		#embed "../../../other/UprightPianoKW.sf2"
	};
	extern const std::size_t upright_piano_kw_sf2_size = sizeof(upright_piano_kw_sf2);
}
