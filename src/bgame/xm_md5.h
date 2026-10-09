#ifndef BGAME_XM_MD5_H
#define BGAME_XM_MD5_H

#include <cstddef>
#include <cstdint>
#include <cstring>

// MD5 identifies known module files; it is not used for authentication.
namespace xm_md5 {

class MD5 {
public:
    MD5() { reset(); }

    void reset() {
        state_[0] = 0x67452301u;
        state_[1] = 0xefcdab89u;
        state_[2] = 0x98badcfeu;
        state_[3] = 0x10325476u;
        bytes_ = 0;
        buffered_ = 0;
    }

    void update(const uint8_t* data, size_t size) {
        if (!size) return;
        bytes_ += static_cast<uint64_t>(size);
        if (buffered_) {
            const size_t take = size < 64 - buffered_ ? size : 64 - buffered_;
            if (take) std::memcpy(buffer_ + buffered_, data, take);
            buffered_ += take;
            data += take;
            size -= take;
            if (buffered_ == 64) {
                transform(buffer_);
                buffered_ = 0;
            }
        }
        while (size >= 64) {
            transform(data);
            data += 64;
            size -= 64;
        }
        if (size) {
            std::memcpy(buffer_, data, size);
            buffered_ = size;
        }
    }

    void finalize(uint8_t digest[16]) {
        const uint64_t bits = bytes_ * 8;
        uint8_t padding[64] = { 0x80 };
        const size_t paddingSize = buffered_ < 56 ? 56 - buffered_ : 120 - buffered_;
        update(padding, paddingSize);
        uint8_t length[8];
        for (size_t i = 0; i < 8; ++i) length[i] = static_cast<uint8_t>(bits >> (8 * i));
        update(length, sizeof(length));
        for (size_t i = 0; i < 16; ++i) {
            digest[i] = static_cast<uint8_t>(state_[i / 4] >> (8 * (i % 4)));
        }
    }

private:
    uint32_t state_[4];
    uint64_t bytes_;
    uint8_t buffer_[64];
    size_t buffered_;

    void transform(const uint8_t* data) {
        static const uint32_t constants[64] = {
            0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
            0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
            0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
            0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
            0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
            0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
            0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
            0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
            0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
            0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
            0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
            0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
            0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
            0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
            0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
            0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u
        };
        static const unsigned rotations[64] = {
            7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
            5,9,14,20, 5,9,14,20, 5,9,14,20, 5,9,14,20,
            4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
            6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
        };
        uint32_t words[16];
        for (size_t i = 0; i < 16; ++i) {
            const uint8_t* p = data + i * 4;
            words[i] = uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
                       (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
        }
        uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];
        for (unsigned i = 0; i < 64; ++i) {
            uint32_t f;
            unsigned word;
            if (i < 16) { f = (b & c) | (~b & d); word = i; }
            else if (i < 32) { f = (d & b) | (~d & c); word = (5 * i + 1) & 15; }
            else if (i < 48) { f = b ^ c ^ d; word = (3 * i + 5) & 15; }
            else { f = c ^ (b | ~d); word = (7 * i) & 15; }
            const uint32_t sum = a + f + constants[i] + words[word];
            a = d;
            d = c;
            c = b;
            b += (sum << rotations[i]) | (sum >> (32 - rotations[i]));
        }
        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
    }
};

inline void toHex(const uint8_t digest[16], char output[33]) {
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < 16; ++i) {
        output[2 * i] = hex[digest[i] >> 4];
        output[2 * i + 1] = hex[digest[i] & 15];
    }
    output[32] = '\0';
}

} // namespace xm_md5

#endif // BGAME_XM_MD5_H
