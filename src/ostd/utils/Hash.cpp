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

#include "Hash.hpp"
#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <iomanip>

// Rotation amounts
namespace {
    constexpr u32 S11 = 7;
    constexpr u32 S12 = 12;
    constexpr u32 S13 = 17;
    constexpr u32 S14 = 22;
    constexpr u32 S21 = 5;
    constexpr u32 S22 = 9;
    constexpr u32 S23 = 14;
    constexpr u32 S24 = 20;
    constexpr u32 S31 = 4;
    constexpr u32 S32 = 11;
    constexpr u32 S33 = 16;
    constexpr u32 S34 = 23;
    constexpr u32 S41 = 6;
    constexpr u32 S42 = 10;
    constexpr u32 S43 = 15;
    constexpr u32 S44 = 21;

    inline u32 F(u32 x, u32 y, u32 z) { return (x & y) | (~x & z); }
    inline u32 G(u32 x, u32 y, u32 z) { return (x & z) | (y & ~z); }
    inline u32 H(u32 x, u32 y, u32 z) { return x ^ y ^ z; }
    inline u32 I(u32 x, u32 y, u32 z) { return y ^ (x | ~z); }

    inline u32 rotate_left(u32 x, u32 n) { return (x << n) | (x >> (32 - n)); }

    inline void FF(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac) {
        a += F(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
    inline void GG(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac) {
        a += G(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
    inline void HH(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac) {
        a += H(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
    inline void II(u32& a, u32 b, u32 c, u32 d, u32 x, u32 s, u32 ac) {
        a += I(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
}

String ostd::Hash::md5(const String& _str) {
	std::string str = _str.cpp_str();
    // Initial state (RFC 1321)
    u32 state[4] = {
        0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u
    };

    // Process input in 64-byte blocks
    const u8* input = reinterpret_cast<const u8*>(str.data());
    size_t inputLen = str.size();

    // Process full 64-byte chunks
    size_t i = 0;
    for (; i + 63 < inputLen; i += 64) {
        __md5_transform(input + i, state);
    }

    // Buffer for remaining bytes + padding + length
    u8 buffer[64];
    size_t rem = inputLen - i;
    std::memset(buffer, 0, sizeof(buffer));
    if (rem > 0) {
        std::memcpy(buffer, input + i, rem);
    }

    // Append 0x80
    buffer[rem++] = 0x80;

    if (rem > 56) {
        // Not enough space for length: pad and process this block
        std::memset(buffer + rem, 0, 64 - rem);
        __md5_transform(buffer, state);
        // New block with zeros up to 56
        std::memset(buffer, 0, 56);
    } else {
        // Pad zeros up to 56
        std::memset(buffer + rem, 0, 56 - rem);
    }

    // Append original length in bits, little-endian
    u64 bitLen = cast<u64>(inputLen) * 8ull;
    for (i32 j = 0; j < 8; ++j) {
        buffer[56 + j] = cast<u8>((bitLen >> (8 * j)) & 0xFF);
    }

    // Final transform
    __md5_transform(buffer, state);

    // Output digest (little-endian)
    u8 digest[16];
    __md5_encode(digest, state, 16);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (i32 j = 0; j < 16; ++j) {
        oss << std::setw(2) << cast<u32>(digest[j]);
    }
    return oss.str();
}

void ostd::Hash::__md5_transform(const u8 block[64], u32 state[4]) {
    u32 a = state[0];
    u32 b = state[1];
    u32 c = state[2];
    u32 d = state[3];
    u32 x[16];

    __md5_decode(x, block, 64);

    // Round 1
    FF(a, b, c, d, x[ 0], S11, 0xd76aa478); // 1
    FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); // 2
    FF(c, d, a, b, x[ 2], S13, 0x242070db); // 3
    FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); // 4
    FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); // 5
    FF(d, a, b, c, x[ 5], S12, 0x4787c62a); // 6
    FF(c, d, a, b, x[ 6], S13, 0xa8304613); // 7
    FF(b, c, d, a, x[ 7], S14, 0xfd469501); // 8
    FF(a, b, c, d, x[ 8], S11, 0x698098d8); // 9
    FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); // 10
    FF(c, d, a, b, x[10], S13, 0xffff5bb1); // 11
    FF(b, c, d, a, x[11], S14, 0x895cd7be); // 12
    FF(a, b, c, d, x[12], S11, 0x6b901122); // 13
    FF(d, a, b, c, x[13], S12, 0xfd987193); // 14
    FF(c, d, a, b, x[14], S13, 0xa679438e); // 15
    FF(b, c, d, a, x[15], S14, 0x49b40821); // 16

    // Round 2
    GG(a, b, c, d, x[ 1], S21, 0xf61e2562); // 17
    GG(d, a, b, c, x[ 6], S22, 0xc040b340); // 18
    GG(c, d, a, b, x[11], S23, 0x265e5a51); // 19
    GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); // 20
    GG(a, b, c, d, x[ 5], S21, 0xd62f105d); // 21
    GG(d, a, b, c, x[10], S22, 0x02441453); // 22
    GG(c, d, a, b, x[15], S23, 0xd8a1e681); // 23
    GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); // 24
    GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); // 25
    GG(d, a, b, c, x[14], S22, 0xc33707d6); // 26
    GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); // 27
    GG(b, c, d, a, x[ 8], S24, 0x455a14ed); // 28
    GG(a, b, c, d, x[13], S21, 0xa9e3e905); // 29
    GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); // 30
    GG(c, d, a, b, x[ 7], S23, 0x676f02d9); // 31
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); // 32

    // Round 3
    HH(a, b, c, d, x[ 5], S31, 0xfffa3942); // 33
    HH(d, a, b, c, x[ 8], S32, 0x8771f681); // 34
    HH(c, d, a, b, x[11], S33, 0x6d9d6122); // 35
    HH(b, c, d, a, x[14], S34, 0xfde5380c); // 36
    HH(a, b, c, d, x[ 1], S31, 0xa4beea44); // 37
    HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); // 38
    HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); // 39
    HH(b, c, d, a, x[10], S34, 0xbebfbc70); // 40
    HH(a, b, c, d, x[13], S31, 0x289b7ec6); // 41
    HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); // 42
    HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); // 43
    HH(b, c, d, a, x[ 6], S34, 0x04881d05); // 44
    HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); // 45
    HH(d, a, b, c, x[12], S32, 0xe6db99e5); // 46
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8); // 47
    HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); // 48

    // Round 4
    II(a, b, c, d, x[ 0], S41, 0xf4292244); // 49
    II(d, a, b, c, x[ 7], S42, 0x432aff97); // 50
    II(c, d, a, b, x[14], S43, 0xab9423a7); // 51
    II(b, c, d, a, x[ 5], S44, 0xfc93a039); // 52
    II(a, b, c, d, x[12], S41, 0x655b59c3); // 53
    II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); // 54
    II(c, d, a, b, x[10], S43, 0xffeff47d); // 55
    II(b, c, d, a, x[ 1], S44, 0x85845dd1); // 56
    II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); // 57
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0); // 58
    II(c, d, a, b, x[ 6], S43, 0xa3014314); // 59
    II(b, c, d, a, x[13], S44, 0x4e0811a1); // 60
    II(a, b, c, d, x[ 4], S41, 0xf7537e82); // 61
    II(d, a, b, c, x[11], S42, 0xbd3af235); // 62
    II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); // 63
    II(b, c, d, a, x[ 9], S44, 0xeb86d391); // 64

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

void ostd::Hash::__md5_encode(u8* output, const u32* input, size_t len) {
    // Convert 32-bit words to bytes (little-endian)
    for (size_t i = 0, j = 0; j < len; ++i, j += 4) {
        output[j + 0] = cast<u8>( input[i]        & 0xFF);
        output[j + 1] = cast<u8>((input[i] >> 8)  & 0xFF);
        output[j + 2] = cast<u8>((input[i] >> 16) & 0xFF);
        output[j + 3] = cast<u8>((input[i] >> 24) & 0xFF);
    }
}

void ostd::Hash::__md5_decode(u32* output, const u8* input, size_t len) {
    // Convert bytes to 32-bit words (little-endian)
    for (size_t i = 0, j = 0; j < len; ++i, j += 4) {
        output[i] =  cast<u32>(input[j + 0])
        | (cast<u32>(input[j + 1]) << 8)
        | (cast<u32>(input[j + 2]) << 16)
        | (cast<u32>(input[j + 3]) << 24);
    }
}
