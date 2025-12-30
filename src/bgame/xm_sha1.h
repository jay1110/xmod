#ifndef BGAME_XM_SHA1_H
#define BGAME_XM_SHA1_H

#include <cstdint>
#include <cstring>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// Simple SHA1 implementation for authentication
///////////////////////////////////////////////////////////////////////////////

namespace xm_sha1 {

class SHA1 {
private:
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];

    inline uint32_t rol(uint32_t value, uint32_t bits) {
        return (value << bits) | (value >> (32 - bits));
    }

    inline uint32_t blk0(uint32_t* block, uint32_t i) {
        return block[i];
    }

    inline uint32_t blk(uint32_t* block, uint32_t i) {
        uint32_t val = rol(block[(i+13)&15] ^ block[(i+8)&15] ^ block[(i+2)&15] ^ block[i], 1);
        block[i] = val;
        return val;
    }

    inline void R0(uint32_t* block, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, uint32_t i) {
        z += ((w&(x^y))^y) + blk0(block, i) + 0x5A827999 + rol(v, 5);
        w = rol(w, 30);
    }

    inline void R1(uint32_t* block, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, uint32_t i) {
        z += ((w&(x^y))^y) + blk(block, i) + 0x5A827999 + rol(v, 5);
        w = rol(w, 30);
    }

    inline void R2(uint32_t* block, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, uint32_t i) {
        z += (w^x^y) + blk(block, i) + 0x6ED9EBA1 + rol(v, 5);
        w = rol(w, 30);
    }

    inline void R3(uint32_t* block, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, uint32_t i) {
        z += (((w|x)&y)|(w&x)) + blk(block, i) + 0x8F1BBCDC + rol(v, 5);
        w = rol(w, 30);
    }

    inline void R4(uint32_t* block, uint32_t v, uint32_t& w, uint32_t x, uint32_t y, uint32_t& z, uint32_t i) {
        z += (w^x^y) + blk(block, i) + 0xCA62C1D6 + rol(v, 5);
        w = rol(w, 30);
    }

    void transform(const uint8_t buffer[64]) {
        uint32_t a = state[0];
        uint32_t b = state[1];
        uint32_t c = state[2];
        uint32_t d = state[3];
        uint32_t e = state[4];
        uint32_t block[16];

        memcpy(block, buffer, 64);

        R0(block, a, b, c, d, e, 0); R0(block, e, a, b, c, d, 1); R0(block, d, e, a, b, c, 2); R0(block, c, d, e, a, b, 3);
        R0(block, b, c, d, e, a, 4); R0(block, a, b, c, d, e, 5); R0(block, e, a, b, c, d, 6); R0(block, d, e, a, b, c, 7);
        R0(block, c, d, e, a, b, 8); R0(block, b, c, d, e, a, 9); R0(block, a, b, c, d, e, 10); R0(block, e, a, b, c, d, 11);
        R0(block, d, e, a, b, c, 12); R0(block, c, d, e, a, b, 13); R0(block, b, c, d, e, a, 14); R0(block, a, b, c, d, e, 15);
        R1(block, e, a, b, c, d, 0); R1(block, d, e, a, b, c, 1); R1(block, c, d, e, a, b, 2); R1(block, b, c, d, e, a, 3);
        R2(block, a, b, c, d, e, 4); R2(block, e, a, b, c, d, 5); R2(block, d, e, a, b, c, 6); R2(block, c, d, e, a, b, 7);
        R2(block, b, c, d, e, a, 8); R2(block, a, b, c, d, e, 9); R2(block, e, a, b, c, d, 10); R2(block, d, e, a, b, c, 11);
        R2(block, c, d, e, a, b, 12); R2(block, b, c, d, e, a, 13); R2(block, a, b, c, d, e, 14); R2(block, e, a, b, c, d, 15);
        R2(block, d, e, a, b, c, 0); R2(block, c, d, e, a, b, 1); R2(block, b, c, d, e, a, 2); R2(block, a, b, c, d, e, 3);
        R2(block, e, a, b, c, d, 4); R2(block, d, e, a, b, c, 5); R2(block, c, d, e, a, b, 6); R2(block, b, c, d, e, a, 7);
        R3(block, a, b, c, d, e, 8); R3(block, e, a, b, c, d, 9); R3(block, d, e, a, b, c, 10); R3(block, c, d, e, a, b, 11);
        R3(block, b, c, d, e, a, 12); R3(block, a, b, c, d, e, 13); R3(block, e, a, b, c, d, 14); R3(block, d, e, a, b, c, 15);
        R3(block, c, d, e, a, b, 0); R3(block, b, c, d, e, a, 1); R3(block, a, b, c, d, e, 2); R3(block, e, a, b, c, d, 3);
        R3(block, d, e, a, b, c, 4); R3(block, c, d, e, a, b, 5); R3(block, b, c, d, e, a, 6); R3(block, a, b, c, d, e, 7);
        R4(block, e, a, b, c, d, 8); R4(block, d, e, a, b, c, 9); R4(block, c, d, e, a, b, 10); R4(block, b, c, d, e, a, 11);
        R4(block, a, b, c, d, e, 12); R4(block, e, a, b, c, d, 13); R4(block, d, e, a, b, c, 14); R4(block, c, d, e, a, b, 15);

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
    }

public:
    SHA1() {
        reset();
    }

    void reset() {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        state[4] = 0xC3D2E1F0;
        count[0] = count[1] = 0;
    }

    void update(const uint8_t* data, size_t len) {
        uint32_t i, j;

        j = (count[0] >> 3) & 63;
        if ((count[0] += len << 3) < (len << 3)) count[1]++;
        count[1] += (len >> 29);

        if ((j + len) > 63) {
            i = 64 - j;
            memcpy(&buffer[j], data, i);
            transform(buffer);
            for (; i + 63 < len; i += 64) {
                transform(&data[i]);
            }
            j = 0;
        } else {
            i = 0;
        }

        memcpy(&buffer[j], &data[i], len - i);
    }

    void finalize(uint8_t digest[20]) {
        uint8_t finalcount[8];
        for (unsigned i = 0; i < 8; i++) {
            finalcount[i] = (uint8_t)((count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
        }

        uint8_t c = 0200;
        update(&c, 1);
        while ((count[0] & 504) != 448) {
            c = 0000;
            update(&c, 1);
        }
        update(finalcount, 8);

        for (unsigned i = 0; i < 20; i++) {
            digest[i] = (uint8_t)((state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
        }
    }
};

inline void toHex(const uint8_t* data, size_t len, char* output) {
    const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        output[i * 2] = hex[(data[i] >> 4) & 0xF];
        output[i * 2 + 1] = hex[data[i] & 0xF];
    }
    output[len * 2] = '\0';
}

inline std::string hashString(const std::string& input) {
    SHA1 sha1;
    sha1.update((const uint8_t*)input.c_str(), input.length());
    uint8_t digest[20];
    sha1.finalize(digest);
    char hexOutput[41];
    toHex(digest, 20, hexOutput);
    return std::string(hexOutput);
}

} // namespace xm_sha1

#endif // BGAME_XM_SHA1_H
