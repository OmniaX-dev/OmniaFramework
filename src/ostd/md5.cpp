#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <iomanip>

#include "Utils.hpp"

// Rotation amounts
namespace {
    constexpr uint32_t S11 = 7;
    constexpr uint32_t S12 = 12;
    constexpr uint32_t S13 = 17;
    constexpr uint32_t S14 = 22;
    constexpr uint32_t S21 = 5;
    constexpr uint32_t S22 = 9;
    constexpr uint32_t S23 = 14;
    constexpr uint32_t S24 = 20;
    constexpr uint32_t S31 = 4;
    constexpr uint32_t S32 = 11;
    constexpr uint32_t S33 = 16;
    constexpr uint32_t S34 = 23;
    constexpr uint32_t S41 = 6;
    constexpr uint32_t S42 = 10;
    constexpr uint32_t S43 = 15;
    constexpr uint32_t S44 = 21;

    inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
    inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
    inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
    inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }

    inline uint32_t rotate_left(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }

    inline void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
        a += F(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
    inline void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
        a += G(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
    inline void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
        a += H(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
    inline void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) {
        a += I(b, c, d) + x + ac;
        a = rotate_left(a, s);
        a += b;
    }
}

ostd::String ostd::Utils::md5(const ostd::String& _str) {
	std::string str = _str.cpp_str();
    // Initial state (RFC 1321)
    uint32_t state[4] = {
        0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u
    };

    // Process input in 64-byte blocks
    const uint8_t* input = reinterpret_cast<const uint8_t*>(str.data());
    size_t inputLen = str.size();

    // Process full 64-byte chunks
    size_t i = 0;
    for (; i + 63 < inputLen; i += 64) {
        transform(input + i, state);
    }

    // Buffer for remaining bytes + padding + length
    uint8_t buffer[64];
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
        transform(buffer, state);
        // New block with zeros up to 56
        std::memset(buffer, 0, 56);
    } else {
        // Pad zeros up to 56
        std::memset(buffer + rem, 0, 56 - rem);
    }

    // Append original length in bits, little-endian
    uint64_t bitLen = static_cast<uint64_t>(inputLen) * 8ull;
    for (int j = 0; j < 8; ++j) {
        buffer[56 + j] = static_cast<uint8_t>((bitLen >> (8 * j)) & 0xFF);
    }

    // Final transform
    transform(buffer, state);

    // Output digest (little-endian)
    uint8_t digest[16];
    encode(digest, state, 16);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int j = 0; j < 16; ++j) {
        oss << std::setw(2) << static_cast<unsigned int>(digest[j]);
    }
    return oss.str();
}

void ostd::Utils::transform(const uint8_t block[64], uint32_t state[4]) {
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t x[16];

    decode(x, block, 64);

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

void ostd::Utils::encode(uint8_t* output, const uint32_t* input, size_t len) {
    // Convert 32-bit words to bytes (little-endian)
    for (size_t i = 0, j = 0; j < len; ++i, j += 4) {
        output[j + 0] = static_cast<uint8_t>( input[i]        & 0xFF);
        output[j + 1] = static_cast<uint8_t>((input[i] >> 8)  & 0xFF);
        output[j + 2] = static_cast<uint8_t>((input[i] >> 16) & 0xFF);
        output[j + 3] = static_cast<uint8_t>((input[i] >> 24) & 0xFF);
    }
}

void ostd::Utils::decode(uint32_t* output, const uint8_t* input, size_t len) {
    // Convert bytes to 32-bit words (little-endian)
    for (size_t i = 0, j = 0; j < len; ++i, j += 4) {
        output[i] =  static_cast<uint32_t>(input[j + 0])
                   | (static_cast<uint32_t>(input[j + 1]) << 8)
                   | (static_cast<uint32_t>(input[j + 2]) << 16)
                   | (static_cast<uint32_t>(input[j + 3]) << 24);
    }
}