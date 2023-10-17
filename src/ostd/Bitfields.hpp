#ifndef __BITFIELDS_HPP__
#define __BITFIELDS_HPP__

#include <cstdint>

#define get_bit(__bit_field, __bit_n		) __bit_field.bits.b##__bit_n
#define set_bit(__bit_field, __bit_n		) __bit_field.bits.b##__bit_n = true
#define clr_bit(__bit_field, __bit_n		) __bit_field.bits.b##__bit_n = false
#define tgl_bit(__bit_field, __bit_n		) __bit_field.bits.b##__bit_n = !__bit_field.bits.b##__bit_n
#define val_bit(__bit_field, __bit_n, __val	) __bit_field.bits.b##__bit_n = __val

namespace ostd
{
	union BitField_8
	{
		uint8_t value = 0;
		struct {
			bool b0 : 1;
			bool b1 : 1;
			bool b2 : 1;
			bool b3 : 1;
			bool b4 : 1;
			bool b5 : 1;
			bool b6 : 1;
			bool b7 : 1;
		} bits;
	};
	union BitField_16
	{
		uint16_t value = 0;
		struct {
			bool b0 : 1;
			bool b1 : 1;
			bool b2 : 1;
			bool b3 : 1;
			bool b4 : 1;
			bool b5 : 1;
			bool b6 : 1;
			bool b7 : 1;
			bool b8 : 1;
			bool b9 : 1;
			bool b10 : 1;
			bool b11 : 1;
			bool b12 : 1;
			bool b13 : 1;
			bool b14 : 1;
			bool b15 : 1;
		} bits;
	};
	union BitField_32 
	{
		uint32_t value = 0;
		struct {
			bool b0 : 1;
			bool b1 : 1;
			bool b2 : 1;
			bool b3 : 1;
			bool b4 : 1;
			bool b5 : 1;
			bool b6 : 1;
			bool b7 : 1;
			bool b8 : 1;
			bool b9 : 1;
			bool b10 : 1;
			bool b11 : 1;
			bool b12 : 1;
			bool b13 : 1;
			bool b14 : 1;
			bool b15 : 1;
			bool b16 : 1;
			bool b17 : 1;
			bool b18 : 1;
			bool b19 : 1;
			bool b20 : 1;
			bool b21 : 1;
			bool b22 : 1;
			bool b23 : 1;
			bool b24 : 1;
			bool b25 : 1;
			bool b26 : 1;
			bool b27 : 1;
			bool b28 : 1;
			bool b29 : 1;
			bool b30 : 1;
			bool b31 : 1;
		} bits;
	};
	union BitField_64
	{
		uint64_t value = 0;
		struct {
			bool b0 : 1;
			bool b1 : 1;
			bool b2 : 1;
			bool b3 : 1;
			bool b4 : 1;
			bool b5 : 1;
			bool b6 : 1;
			bool b7 : 1;
			bool b8 : 1;
			bool b9 : 1;
			bool b10 : 1;
			bool b11 : 1;
			bool b12 : 1;
			bool b13 : 1;
			bool b14 : 1;
			bool b15 : 1;
			bool b16 : 1;
			bool b17 : 1;
			bool b18 : 1;
			bool b19 : 1;
			bool b20 : 1;
			bool b21 : 1;
			bool b22 : 1;
			bool b23 : 1;
			bool b24 : 1;
			bool b25 : 1;
			bool b26 : 1;
			bool b27 : 1;
			bool b28 : 1;
			bool b29 : 1;
			bool b30 : 1;
			bool b31 : 1;
			bool b32 : 1;
			bool b33 : 1;
			bool b34 : 1;
			bool b35 : 1;
			bool b36 : 1;
			bool b37 : 1;
			bool b38 : 1;
			bool b39 : 1;
			bool b40 : 1;
			bool b41 : 1;
			bool b42 : 1;
			bool b43 : 1;
			bool b44 : 1;
			bool b45 : 1;
			bool b46 : 1;
			bool b47 : 1;
			bool b48 : 1;
			bool b49 : 1;
			bool b50 : 1;
			bool b51 : 1;
			bool b52 : 1;
			bool b53 : 1;
			bool b54 : 1;
			bool b55 : 1;
			bool b56 : 1;
			bool b57 : 1;
			bool b58 : 1;
			bool b59 : 1;
			bool b60 : 1;
			bool b61 : 1;
			bool b62 : 1;
			bool b63 : 1;
		} bits;
	} ;

	class Bits
	{
		public:
			//8-Bit field
			static inline bool get(const BitField_8& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: return get_bit(bf, 0);
					case 1: return get_bit(bf, 1);
					case 2: return get_bit(bf, 2);
					case 3: return get_bit(bf, 3);
					case 4: return get_bit(bf, 4);
					case 5: return get_bit(bf, 5);
					case 6: return get_bit(bf, 6);
					case 7: return get_bit(bf, 7);
					default: return false;
				}
			}
			static inline void set(BitField_8& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: set_bit(bf, 0); break;
					case 1: set_bit(bf, 1); break;
					case 2: set_bit(bf, 2); break;
					case 3: set_bit(bf, 3); break;
					case 4: set_bit(bf, 4); break;
					case 5: set_bit(bf, 5); break;
					case 6: set_bit(bf, 6); break;
					case 7: set_bit(bf, 7); break;
					default: break;
				}
			}
			static inline void clr(BitField_8& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: clr_bit(bf, 0); break;
					case 1: clr_bit(bf, 1); break;
					case 2: clr_bit(bf, 2); break;
					case 3: clr_bit(bf, 3); break;
					case 4: clr_bit(bf, 4); break;
					case 5: clr_bit(bf, 5); break;
					case 6: clr_bit(bf, 6); break;
					case 7: clr_bit(bf, 7); break;
					default: break;
				}
			}
			static inline void tgl(BitField_8& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: tgl_bit(bf, 0); break;
					case 1: tgl_bit(bf, 1); break;
					case 2: tgl_bit(bf, 2); break;
					case 3: tgl_bit(bf, 3); break;
					case 4: tgl_bit(bf, 4); break;
					case 5: tgl_bit(bf, 5); break;
					case 6: tgl_bit(bf, 6); break;
					case 7: tgl_bit(bf, 7); break;
					default: break;
				}
			}
			static inline void val(BitField_8& bf, const uint8_t& bit, bool value)
			{
				switch (bit)
				{
					case 0: val_bit(bf, 0, value); break;
					case 1: val_bit(bf, 1, value); break;
					case 2: val_bit(bf, 2, value); break;
					case 3: val_bit(bf, 3, value); break;
					case 4: val_bit(bf, 4, value); break;
					case 5: val_bit(bf, 5, value); break;
					case 6: val_bit(bf, 6, value); break;
					case 7: val_bit(bf, 7, value); break;
					default: break;
				}
			}
	
			//16-Bit field
			static inline bool get(const BitField_16& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: return get_bit(bf, 0);
					case 1: return get_bit(bf, 1);
					case 2: return get_bit(bf, 2);
					case 3: return get_bit(bf, 3);
					case 4: return get_bit(bf, 4);
					case 5: return get_bit(bf, 5);
					case 6: return get_bit(bf, 6);
					case 7: return get_bit(bf, 7);
					case 8: return get_bit(bf, 8);
					case 9: return get_bit(bf, 9);
					case 10: return get_bit(bf, 10);
					case 11: return get_bit(bf, 11);
					case 12: return get_bit(bf, 12);
					case 13: return get_bit(bf, 13);
					case 14: return get_bit(bf, 14);
					case 15: return get_bit(bf, 15);
					default: return false;
				}
			}
			static inline void set(BitField_16& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: set_bit(bf, 0); break;
					case 1: set_bit(bf, 1); break;
					case 2: set_bit(bf, 2); break;
					case 3: set_bit(bf, 3); break;
					case 4: set_bit(bf, 4); break;
					case 5: set_bit(bf, 5); break;
					case 6: set_bit(bf, 6); break;
					case 7: set_bit(bf, 7); break;
					case 8: set_bit(bf, 8); break;
					case 9: set_bit(bf, 9); break;
					case 10: set_bit(bf, 10); break;
					case 11: set_bit(bf, 11); break;
					case 12: set_bit(bf, 12); break;
					case 13: set_bit(bf, 13); break;
					case 14: set_bit(bf, 14); break;
					case 15: set_bit(bf, 15); break;
					default: break;
				}
			}
			static inline void clr(BitField_16& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: clr_bit(bf, 0); break;
					case 1: clr_bit(bf, 1); break;
					case 2: clr_bit(bf, 2); break;
					case 3: clr_bit(bf, 3); break;
					case 4: clr_bit(bf, 4); break;
					case 5: clr_bit(bf, 5); break;
					case 6: clr_bit(bf, 6); break;
					case 7: clr_bit(bf, 7); break;
					case 8: clr_bit(bf, 8); break;
					case 9: clr_bit(bf, 9); break;
					case 10: clr_bit(bf, 10); break;
					case 11: clr_bit(bf, 11); break;
					case 12: clr_bit(bf, 12); break;
					case 13: clr_bit(bf, 13); break;
					case 14: clr_bit(bf, 14); break;
					case 15: clr_bit(bf, 15); break;
					default: break;
				}
			}
			static inline void tgl(BitField_16& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: tgl_bit(bf, 0); break;
					case 1: tgl_bit(bf, 1); break;
					case 2: tgl_bit(bf, 2); break;
					case 3: tgl_bit(bf, 3); break;
					case 4: tgl_bit(bf, 4); break;
					case 5: tgl_bit(bf, 5); break;
					case 6: tgl_bit(bf, 6); break;
					case 7: tgl_bit(bf, 7); break;
					case 8: tgl_bit(bf, 8); break;
					case 9: tgl_bit(bf, 9); break;
					case 10: tgl_bit(bf, 10); break;
					case 11: tgl_bit(bf, 11); break;
					case 12: tgl_bit(bf, 12); break;
					case 13: tgl_bit(bf, 13); break;
					case 14: tgl_bit(bf, 14); break;
					case 15: tgl_bit(bf, 15); break;
					default: break;
				}
			}
			static inline void val(BitField_16& bf, const uint8_t& bit, bool value)
			{
				switch (bit)
				{
					case 0: val_bit(bf, 0, value); break;
					case 1: val_bit(bf, 1, value); break;
					case 2: val_bit(bf, 2, value); break;
					case 3: val_bit(bf, 3, value); break;
					case 4: val_bit(bf, 4, value); break;
					case 5: val_bit(bf, 5, value); break;
					case 6: val_bit(bf, 6, value); break;
					case 7: val_bit(bf, 7, value); break;
					case 8: val_bit(bf, 8, value); break;
					case 9: val_bit(bf, 9, value); break;
					case 10: val_bit(bf, 10, value); break;
					case 11: val_bit(bf, 11, value); break;
					case 12: val_bit(bf, 12, value); break;
					case 13: val_bit(bf, 13, value); break;
					case 14: val_bit(bf, 14, value); break;
					case 15: val_bit(bf, 15, value); break;
					default: break;
				}
			}
	
			//32-Bit field
			static inline bool get(const BitField_32& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: return get_bit(bf, 0);
					case 1: return get_bit(bf, 1);
					case 2: return get_bit(bf, 2);
					case 3: return get_bit(bf, 3);
					case 4: return get_bit(bf, 4);
					case 5: return get_bit(bf, 5);
					case 6: return get_bit(bf, 6);
					case 7: return get_bit(bf, 7);
					case 8: return get_bit(bf, 8);
					case 9: return get_bit(bf, 9);
					case 10: return get_bit(bf, 10);
					case 11: return get_bit(bf, 11);
					case 12: return get_bit(bf, 12);
					case 13: return get_bit(bf, 13);
					case 14: return get_bit(bf, 14);
					case 15: return get_bit(bf, 15);
					case 16: return get_bit(bf, 16);
					case 17: return get_bit(bf, 17);
					case 18: return get_bit(bf, 18);
					case 19: return get_bit(bf, 19);
					case 20: return get_bit(bf, 20);
					case 21: return get_bit(bf, 21);
					case 22: return get_bit(bf, 22);
					case 23: return get_bit(bf, 23);
					case 24: return get_bit(bf, 24);
					case 25: return get_bit(bf, 25);
					case 26: return get_bit(bf, 26);
					case 27: return get_bit(bf, 27);
					case 28: return get_bit(bf, 28);
					case 29: return get_bit(bf, 29);
					case 30: return get_bit(bf, 30);
					case 31: return get_bit(bf, 31);
					default: return false;
				}
			}
			static inline void set(BitField_32& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: set_bit(bf, 0); break;
					case 1: set_bit(bf, 1); break;
					case 2: set_bit(bf, 2); break;
					case 3: set_bit(bf, 3); break;
					case 4: set_bit(bf, 4); break;
					case 5: set_bit(bf, 5); break;
					case 6: set_bit(bf, 6); break;
					case 7: set_bit(bf, 7); break;
					case 8: set_bit(bf, 8); break;
					case 9: set_bit(bf, 9); break;
					case 10: set_bit(bf, 10); break;
					case 11: set_bit(bf, 11); break;
					case 12: set_bit(bf, 12); break;
					case 13: set_bit(bf, 13); break;
					case 14: set_bit(bf, 14); break;
					case 15: set_bit(bf, 15); break;
					case 16: set_bit(bf, 16); break;
					case 17: set_bit(bf, 17); break;
					case 18: set_bit(bf, 18); break;
					case 19: set_bit(bf, 19); break;
					case 20: set_bit(bf, 20); break;
					case 21: set_bit(bf, 21); break;
					case 22: set_bit(bf, 22); break;
					case 23: set_bit(bf, 23); break;
					case 24: set_bit(bf, 24); break;
					case 25: set_bit(bf, 25); break;
					case 26: set_bit(bf, 26); break;
					case 27: set_bit(bf, 27); break;
					case 28: set_bit(bf, 28); break;
					case 29: set_bit(bf, 29); break;
					case 30: set_bit(bf, 30); break;
					case 31: set_bit(bf, 31); break;
					default: break;
				}
			}
			static inline void clr(BitField_32& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: clr_bit(bf, 0); break;
					case 1: clr_bit(bf, 1); break;
					case 2: clr_bit(bf, 2); break;
					case 3: clr_bit(bf, 3); break;
					case 4: clr_bit(bf, 4); break;
					case 5: clr_bit(bf, 5); break;
					case 6: clr_bit(bf, 6); break;
					case 7: clr_bit(bf, 7); break;
					case 8: clr_bit(bf, 8); break;
					case 9: clr_bit(bf, 9); break;
					case 10: clr_bit(bf, 10); break;
					case 11: clr_bit(bf, 11); break;
					case 12: clr_bit(bf, 12); break;
					case 13: clr_bit(bf, 13); break;
					case 14: clr_bit(bf, 14); break;
					case 15: clr_bit(bf, 15); break;
					case 16: clr_bit(bf, 16); break;
					case 17: clr_bit(bf, 17); break;
					case 18: clr_bit(bf, 18); break;
					case 19: clr_bit(bf, 19); break;
					case 20: clr_bit(bf, 20); break;
					case 21: clr_bit(bf, 21); break;
					case 22: clr_bit(bf, 22); break;
					case 23: clr_bit(bf, 23); break;
					case 24: clr_bit(bf, 24); break;
					case 25: clr_bit(bf, 25); break;
					case 26: clr_bit(bf, 26); break;
					case 27: clr_bit(bf, 27); break;
					case 28: clr_bit(bf, 28); break;
					case 29: clr_bit(bf, 29); break;
					case 30: clr_bit(bf, 30); break;
					case 31: clr_bit(bf, 31); break;
					default: break;
				}
			}
			static inline void tgl(BitField_32& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: tgl_bit(bf, 0); break;
					case 1: tgl_bit(bf, 1); break;
					case 2: tgl_bit(bf, 2); break;
					case 3: tgl_bit(bf, 3); break;
					case 4: tgl_bit(bf, 4); break;
					case 5: tgl_bit(bf, 5); break;
					case 6: tgl_bit(bf, 6); break;
					case 7: tgl_bit(bf, 7); break;
					case 8: tgl_bit(bf, 8); break;
					case 9: tgl_bit(bf, 9); break;
					case 10: tgl_bit(bf, 10); break;
					case 11: tgl_bit(bf, 11); break;
					case 12: tgl_bit(bf, 12); break;
					case 13: tgl_bit(bf, 13); break;
					case 14: tgl_bit(bf, 14); break;
					case 15: tgl_bit(bf, 15); break;
					case 16: tgl_bit(bf, 16); break;
					case 17: tgl_bit(bf, 17); break;
					case 18: tgl_bit(bf, 18); break;
					case 19: tgl_bit(bf, 19); break;
					case 20: tgl_bit(bf, 20); break;
					case 21: tgl_bit(bf, 21); break;
					case 22: tgl_bit(bf, 22); break;
					case 23: tgl_bit(bf, 23); break;
					case 24: tgl_bit(bf, 24); break;
					case 25: tgl_bit(bf, 25); break;
					case 26: tgl_bit(bf, 26); break;
					case 27: tgl_bit(bf, 27); break;
					case 28: tgl_bit(bf, 28); break;
					case 29: tgl_bit(bf, 29); break;
					case 30: tgl_bit(bf, 30); break;
					case 31: tgl_bit(bf, 31); break;
					default: break;
				}
			}
			static inline void val(BitField_32& bf, const uint8_t& bit, bool value)
			{
				switch (bit)
				{
					case 0: val_bit(bf, 0, value); break;
					case 1: val_bit(bf, 1, value); break;
					case 2: val_bit(bf, 2, value); break;
					case 3: val_bit(bf, 3, value); break;
					case 4: val_bit(bf, 4, value); break;
					case 5: val_bit(bf, 5, value); break;
					case 6: val_bit(bf, 6, value); break;
					case 7: val_bit(bf, 7, value); break;
					case 8: val_bit(bf, 8, value); break;
					case 9: val_bit(bf, 9, value); break;
					case 10: val_bit(bf, 10, value); break;
					case 11: val_bit(bf, 11, value); break;
					case 12: val_bit(bf, 12, value); break;
					case 13: val_bit(bf, 13, value); break;
					case 14: val_bit(bf, 14, value); break;
					case 15: val_bit(bf, 15, value); break;
					case 16: val_bit(bf, 16, value); break;
					case 17: val_bit(bf, 17, value); break;
					case 18: val_bit(bf, 18, value); break;
					case 19: val_bit(bf, 19, value); break;
					case 20: val_bit(bf, 20, value); break;
					case 21: val_bit(bf, 21, value); break;
					case 22: val_bit(bf, 22, value); break;
					case 23: val_bit(bf, 23, value); break;
					case 24: val_bit(bf, 24, value); break;
					case 25: val_bit(bf, 25, value); break;
					case 26: val_bit(bf, 26, value); break;
					case 27: val_bit(bf, 27, value); break;
					case 28: val_bit(bf, 28, value); break;
					case 29: val_bit(bf, 29, value); break;
					case 30: val_bit(bf, 30, value); break;
					case 31: val_bit(bf, 31, value); break;
					default: break;
				}
			}
	
			//64-Bit field
			static inline bool get(const BitField_64& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: return get_bit(bf, 0);
					case 1: return get_bit(bf, 1);
					case 2: return get_bit(bf, 2);
					case 3: return get_bit(bf, 3);
					case 4: return get_bit(bf, 4);
					case 5: return get_bit(bf, 5);
					case 6: return get_bit(bf, 6);
					case 7: return get_bit(bf, 7);
					case 8: return get_bit(bf, 8);
					case 9: return get_bit(bf, 9);
					case 10: return get_bit(bf, 10);
					case 11: return get_bit(bf, 11);
					case 12: return get_bit(bf, 12);
					case 13: return get_bit(bf, 13);
					case 14: return get_bit(bf, 14);
					case 15: return get_bit(bf, 15);
					case 16: return get_bit(bf, 16);
					case 17: return get_bit(bf, 17);
					case 18: return get_bit(bf, 18);
					case 19: return get_bit(bf, 19);
					case 20: return get_bit(bf, 20);
					case 21: return get_bit(bf, 21);
					case 22: return get_bit(bf, 22);
					case 23: return get_bit(bf, 23);
					case 24: return get_bit(bf, 24);
					case 25: return get_bit(bf, 25);
					case 26: return get_bit(bf, 26);
					case 27: return get_bit(bf, 27);
					case 28: return get_bit(bf, 28);
					case 29: return get_bit(bf, 29);
					case 30: return get_bit(bf, 30);
					case 31: return get_bit(bf, 31);
					case 32: return get_bit(bf, 32);
					case 33: return get_bit(bf, 33);
					case 34: return get_bit(bf, 34);
					case 35: return get_bit(bf, 35);
					case 36: return get_bit(bf, 36);
					case 37: return get_bit(bf, 37);
					case 38: return get_bit(bf, 38);
					case 39: return get_bit(bf, 39);
					case 40: return get_bit(bf, 40);
					case 41: return get_bit(bf, 41);
					case 42: return get_bit(bf, 42);
					case 43: return get_bit(bf, 43);
					case 44: return get_bit(bf, 44);
					case 45: return get_bit(bf, 45);
					case 46: return get_bit(bf, 46);
					case 47: return get_bit(bf, 47);
					case 48: return get_bit(bf, 48);
					case 49: return get_bit(bf, 49);
					case 50: return get_bit(bf, 50);
					case 51: return get_bit(bf, 51);
					case 52: return get_bit(bf, 52);
					case 53: return get_bit(bf, 53);
					case 54: return get_bit(bf, 54);
					case 55: return get_bit(bf, 55);
					case 56: return get_bit(bf, 56);
					case 57: return get_bit(bf, 57);
					case 58: return get_bit(bf, 58);
					case 59: return get_bit(bf, 59);
					case 60: return get_bit(bf, 60);
					case 61: return get_bit(bf, 61);
					case 62: return get_bit(bf, 62);
					case 63: return get_bit(bf, 63);
					default: return false;
				}
			}
			static inline void set(BitField_64& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: set_bit(bf, 0); break;
					case 1: set_bit(bf, 1); break;
					case 2: set_bit(bf, 2); break;
					case 3: set_bit(bf, 3); break;
					case 4: set_bit(bf, 4); break;
					case 5: set_bit(bf, 5); break;
					case 6: set_bit(bf, 6); break;
					case 7: set_bit(bf, 7); break;
					case 8: set_bit(bf, 8); break;
					case 9: set_bit(bf, 9); break;
					case 10: set_bit(bf, 10); break;
					case 11: set_bit(bf, 11); break;
					case 12: set_bit(bf, 12); break;
					case 13: set_bit(bf, 13); break;
					case 14: set_bit(bf, 14); break;
					case 15: set_bit(bf, 15); break;
					case 16: set_bit(bf, 16); break;
					case 17: set_bit(bf, 17); break;
					case 18: set_bit(bf, 18); break;
					case 19: set_bit(bf, 19); break;
					case 20: set_bit(bf, 20); break;
					case 21: set_bit(bf, 21); break;
					case 22: set_bit(bf, 22); break;
					case 23: set_bit(bf, 23); break;
					case 24: set_bit(bf, 24); break;
					case 25: set_bit(bf, 25); break;
					case 26: set_bit(bf, 26); break;
					case 27: set_bit(bf, 27); break;
					case 28: set_bit(bf, 28); break;
					case 29: set_bit(bf, 29); break;
					case 30: set_bit(bf, 30); break;
					case 31: set_bit(bf, 31); break;
					case 32: set_bit(bf, 32); break;
					case 33: set_bit(bf, 33); break;
					case 34: set_bit(bf, 34); break;
					case 35: set_bit(bf, 35); break;
					case 36: set_bit(bf, 36); break;
					case 37: set_bit(bf, 37); break;
					case 38: set_bit(bf, 38); break;
					case 39: set_bit(bf, 39); break;
					case 40: set_bit(bf, 40); break;
					case 41: set_bit(bf, 41); break;
					case 42: set_bit(bf, 42); break;
					case 43: set_bit(bf, 43); break;
					case 44: set_bit(bf, 44); break;
					case 45: set_bit(bf, 45); break;
					case 46: set_bit(bf, 46); break;
					case 47: set_bit(bf, 47); break;
					case 48: set_bit(bf, 48); break;
					case 49: set_bit(bf, 49); break;
					case 50: set_bit(bf, 50); break;
					case 51: set_bit(bf, 51); break;
					case 52: set_bit(bf, 52); break;
					case 53: set_bit(bf, 53); break;
					case 54: set_bit(bf, 54); break;
					case 55: set_bit(bf, 55); break;
					case 56: set_bit(bf, 56); break;
					case 57: set_bit(bf, 57); break;
					case 58: set_bit(bf, 58); break;
					case 59: set_bit(bf, 59); break;
					case 60: set_bit(bf, 60); break;
					case 61: set_bit(bf, 61); break;
					case 62: set_bit(bf, 62); break;
					case 63: set_bit(bf, 63); break;
					default: break;
				}
			}
			static inline void clr(BitField_64& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: clr_bit(bf, 0); break;
					case 1: clr_bit(bf, 1); break;
					case 2: clr_bit(bf, 2); break;
					case 3: clr_bit(bf, 3); break;
					case 4: clr_bit(bf, 4); break;
					case 5: clr_bit(bf, 5); break;
					case 6: clr_bit(bf, 6); break;
					case 7: clr_bit(bf, 7); break;
					case 8: clr_bit(bf, 8); break;
					case 9: clr_bit(bf, 9); break;
					case 10: clr_bit(bf, 10); break;
					case 11: clr_bit(bf, 11); break;
					case 12: clr_bit(bf, 12); break;
					case 13: clr_bit(bf, 13); break;
					case 14: clr_bit(bf, 14); break;
					case 15: clr_bit(bf, 15); break;
					case 16: clr_bit(bf, 16); break;
					case 17: clr_bit(bf, 17); break;
					case 18: clr_bit(bf, 18); break;
					case 19: clr_bit(bf, 19); break;
					case 20: clr_bit(bf, 20); break;
					case 21: clr_bit(bf, 21); break;
					case 22: clr_bit(bf, 22); break;
					case 23: clr_bit(bf, 23); break;
					case 24: clr_bit(bf, 24); break;
					case 25: clr_bit(bf, 25); break;
					case 26: clr_bit(bf, 26); break;
					case 27: clr_bit(bf, 27); break;
					case 28: clr_bit(bf, 28); break;
					case 29: clr_bit(bf, 29); break;
					case 30: clr_bit(bf, 30); break;
					case 31: clr_bit(bf, 31); break;
					case 32: clr_bit(bf, 32); break;
					case 33: clr_bit(bf, 33); break;
					case 34: clr_bit(bf, 34); break;
					case 35: clr_bit(bf, 35); break;
					case 36: clr_bit(bf, 36); break;
					case 37: clr_bit(bf, 37); break;
					case 38: clr_bit(bf, 38); break;
					case 39: clr_bit(bf, 39); break;
					case 40: clr_bit(bf, 40); break;
					case 41: clr_bit(bf, 41); break;
					case 42: clr_bit(bf, 42); break;
					case 43: clr_bit(bf, 43); break;
					case 44: clr_bit(bf, 44); break;
					case 45: clr_bit(bf, 45); break;
					case 46: clr_bit(bf, 46); break;
					case 47: clr_bit(bf, 47); break;
					case 48: clr_bit(bf, 48); break;
					case 49: clr_bit(bf, 49); break;
					case 50: clr_bit(bf, 50); break;
					case 51: clr_bit(bf, 51); break;
					case 52: clr_bit(bf, 52); break;
					case 53: clr_bit(bf, 53); break;
					case 54: clr_bit(bf, 54); break;
					case 55: clr_bit(bf, 55); break;
					case 56: clr_bit(bf, 56); break;
					case 57: clr_bit(bf, 57); break;
					case 58: clr_bit(bf, 58); break;
					case 59: clr_bit(bf, 59); break;
					case 60: clr_bit(bf, 60); break;
					case 61: clr_bit(bf, 61); break;
					case 62: clr_bit(bf, 62); break;
					case 63: clr_bit(bf, 63); break;
					default: break;
				}
			}
			static inline void tgl(BitField_64& bf, const uint8_t& bit)
			{
				switch (bit)
				{
					case 0: tgl_bit(bf, 0); break;
					case 1: tgl_bit(bf, 1); break;
					case 2: tgl_bit(bf, 2); break;
					case 3: tgl_bit(bf, 3); break;
					case 4: tgl_bit(bf, 4); break;
					case 5: tgl_bit(bf, 5); break;
					case 6: tgl_bit(bf, 6); break;
					case 7: tgl_bit(bf, 7); break;
					case 8: tgl_bit(bf, 8); break;
					case 9: tgl_bit(bf, 9); break;
					case 10: tgl_bit(bf, 10); break;
					case 11: tgl_bit(bf, 11); break;
					case 12: tgl_bit(bf, 12); break;
					case 13: tgl_bit(bf, 13); break;
					case 14: tgl_bit(bf, 14); break;
					case 15: tgl_bit(bf, 15); break;
					case 16: tgl_bit(bf, 16); break;
					case 17: tgl_bit(bf, 17); break;
					case 18: tgl_bit(bf, 18); break;
					case 19: tgl_bit(bf, 19); break;
					case 20: tgl_bit(bf, 20); break;
					case 21: tgl_bit(bf, 21); break;
					case 22: tgl_bit(bf, 22); break;
					case 23: tgl_bit(bf, 23); break;
					case 24: tgl_bit(bf, 24); break;
					case 25: tgl_bit(bf, 25); break;
					case 26: tgl_bit(bf, 26); break;
					case 27: tgl_bit(bf, 27); break;
					case 28: tgl_bit(bf, 28); break;
					case 29: tgl_bit(bf, 29); break;
					case 30: tgl_bit(bf, 30); break;
					case 31: tgl_bit(bf, 31); break;
					case 32: tgl_bit(bf, 32); break;
					case 33: tgl_bit(bf, 33); break;
					case 34: tgl_bit(bf, 34); break;
					case 35: tgl_bit(bf, 35); break;
					case 36: tgl_bit(bf, 36); break;
					case 37: tgl_bit(bf, 37); break;
					case 38: tgl_bit(bf, 38); break;
					case 39: tgl_bit(bf, 39); break;
					case 40: tgl_bit(bf, 40); break;
					case 41: tgl_bit(bf, 41); break;
					case 42: tgl_bit(bf, 42); break;
					case 43: tgl_bit(bf, 43); break;
					case 44: tgl_bit(bf, 44); break;
					case 45: tgl_bit(bf, 45); break;
					case 46: tgl_bit(bf, 46); break;
					case 47: tgl_bit(bf, 47); break;
					case 48: tgl_bit(bf, 48); break;
					case 49: tgl_bit(bf, 49); break;
					case 50: tgl_bit(bf, 50); break;
					case 51: tgl_bit(bf, 51); break;
					case 52: tgl_bit(bf, 52); break;
					case 53: tgl_bit(bf, 53); break;
					case 54: tgl_bit(bf, 54); break;
					case 55: tgl_bit(bf, 55); break;
					case 56: tgl_bit(bf, 56); break;
					case 57: tgl_bit(bf, 57); break;
					case 58: tgl_bit(bf, 58); break;
					case 59: tgl_bit(bf, 59); break;
					case 60: tgl_bit(bf, 60); break;
					case 61: tgl_bit(bf, 61); break;
					case 62: tgl_bit(bf, 62); break;
					case 63: tgl_bit(bf, 63); break;
					default: break;
				}
			}
			static inline void val(BitField_64& bf, const uint8_t& bit, bool value)
			{
				switch (bit)
				{
					case 0: val_bit(bf, 0, value); break;
					case 1: val_bit(bf, 1, value); break;
					case 2: val_bit(bf, 2, value); break;
					case 3: val_bit(bf, 3, value); break;
					case 4: val_bit(bf, 4, value); break;
					case 5: val_bit(bf, 5, value); break;
					case 6: val_bit(bf, 6, value); break;
					case 7: val_bit(bf, 7, value); break;
					case 8: val_bit(bf, 8, value); break;
					case 9: val_bit(bf, 9, value); break;
					case 10: val_bit(bf, 10, value); break;
					case 11: val_bit(bf, 11, value); break;
					case 12: val_bit(bf, 12, value); break;
					case 13: val_bit(bf, 13, value); break;
					case 14: val_bit(bf, 14, value); break;
					case 15: val_bit(bf, 15, value); break;
					case 16: val_bit(bf, 16, value); break;
					case 17: val_bit(bf, 17, value); break;
					case 18: val_bit(bf, 18, value); break;
					case 19: val_bit(bf, 19, value); break;
					case 20: val_bit(bf, 20, value); break;
					case 21: val_bit(bf, 21, value); break;
					case 22: val_bit(bf, 22, value); break;
					case 23: val_bit(bf, 23, value); break;
					case 24: val_bit(bf, 24, value); break;
					case 25: val_bit(bf, 25, value); break;
					case 26: val_bit(bf, 26, value); break;
					case 27: val_bit(bf, 27, value); break;
					case 28: val_bit(bf, 28, value); break;
					case 29: val_bit(bf, 29, value); break;
					case 30: val_bit(bf, 30, value); break;
					case 31: val_bit(bf, 31, value); break;
					case 32: val_bit(bf, 32, value); break;
					case 33: val_bit(bf, 33, value); break;
					case 34: val_bit(bf, 34, value); break;
					case 35: val_bit(bf, 35, value); break;
					case 36: val_bit(bf, 36, value); break;
					case 37: val_bit(bf, 37, value); break;
					case 38: val_bit(bf, 38, value); break;
					case 39: val_bit(bf, 39, value); break;
					case 40: val_bit(bf, 40, value); break;
					case 41: val_bit(bf, 41, value); break;
					case 42: val_bit(bf, 42, value); break;
					case 43: val_bit(bf, 43, value); break;
					case 44: val_bit(bf, 44, value); break;
					case 45: val_bit(bf, 45, value); break;
					case 46: val_bit(bf, 46, value); break;
					case 47: val_bit(bf, 47, value); break;
					case 48: val_bit(bf, 48, value); break;
					case 49: val_bit(bf, 49, value); break;
					case 50: val_bit(bf, 50, value); break;
					case 51: val_bit(bf, 51, value); break;
					case 52: val_bit(bf, 52, value); break;
					case 53: val_bit(bf, 53, value); break;
					case 54: val_bit(bf, 54, value); break;
					case 55: val_bit(bf, 55, value); break;
					case 56: val_bit(bf, 56, value); break;
					case 57: val_bit(bf, 57, value); break;
					case 58: val_bit(bf, 58, value); break;
					case 59: val_bit(bf, 59, value); break;
					case 60: val_bit(bf, 60, value); break;
					case 61: val_bit(bf, 61, value); break;
					case 62: val_bit(bf, 62, value); break;
					case 63: val_bit(bf, 63, value); break;
					default: break;
				}
			}
	};
} // namesoace ox



#endif