#ifndef H2_HASH_HPP
#define H2_HASH_HPP

/*
    Header-only library of H2 hash
    H2 hash algorithm by Naharashu (me)
    Apache 2.0 License
    Version 1.0 2026-09-24
*/

#include "SMHasher/src/Bitvec.h"
#include <atomic>
#include <cstdint>
#include <span>
#include <vector>
#include <bit>
#include <cstring>

union uint256 {
    struct { uint64_t a, b, c, d; };
    uint64_t v[4];
};


#if defined(_MSC_VER) && defined(_M_X64)
    #include <intrin.h>
#endif

inline uint64_t mum_mix(uint64_t state, uint64_t prime) {
#if defined(__SIZEOF_INT128__) || defined(__clang__)
    unsigned __int128 product = (unsigned __int128)state * prime;
    return (uint64_t)product ^ (uint64_t)(product >> 64) ^ state ^ prime;

#elif defined(_MSC_VER) && defined(_M_X64)
    uint64_t high;
    uint64_t low = _umul128(state, prime, &high);
    return low ^ high ^ state ^ prime;

#else
    uint64_t ha = state >> 32; uint64_t la = (uint32_t)state;
    uint64_t hb = prime >> 32; uint64_t lb = (uint32_t)prime;
    uint64_t rh = ha * hb;
    uint64_t rm0 = ha * lb;
    uint64_t rm1 = hb * la;
    uint64_t rl = la * lb;
    uint64_t t = rl + (rm0 << 32);
    uint64_t c = (t < rl) + (rm0 >> 32);
    uint64_t low = t + (rm1 << 32);
    uint64_t high = rh + c + (rm1 >> 32) + (low < t);
    return low ^ high ^ state ^ prime;
#endif
}


inline uint64_t splitmix(uint64_t state) {
    state = (state ^ (state >> 30)) * 0xBF58476D1CE4E5B9ULL;
    state = (state ^ (state >> 27)) * 0x94D049BB133111EBULL;
    return state ^ (state >> 31);
}





alignas(64) static constexpr unsigned char sbox[256] = 
{
   0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
   0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
   0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
   0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
   0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
   0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
   0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
   0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
   0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
   0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
   0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
   0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
   0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
   0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
   0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
   0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};



inline uint64_t sbox64(uint64_t x) {
    return uint64_t(sbox[ x        & 0xff])       |
          (uint64_t(sbox[(x >>  8) & 0xff]) <<  8) |
          (uint64_t(sbox[(x >> 16) & 0xff]) << 16) |
          (uint64_t(sbox[(x >> 24) & 0xff]) << 24) |
          (uint64_t(sbox[(x >> 32) & 0xff]) << 32) |
          (uint64_t(sbox[(x >> 40) & 0xff]) << 40) |
          (uint64_t(sbox[(x >> 48) & 0xff]) << 48) |
          (uint64_t(sbox[(x >> 56)        ]) << 56);
}




inline constexpr std::uint64_t H2_C1 = 0x9e3779b97f4a7c15ULL;
inline constexpr std::uint64_t H2_C2 = 0xbb67ae8584caa73bULL;
inline constexpr std::uint64_t H2_C3 = 0xb7e151628aed2a6aULL;
inline constexpr std::uint64_t H2_C4 = 0xab1c5ed5da6d8118ULL;

inline constexpr std::uint64_t H2_C5 = 0x6a09e667f3bcc908ULL;
inline constexpr std::uint64_t H2_C6 = 0xA4D94E92C3B4A3D7ULL;
inline constexpr std::uint64_t H2_C7 = 0xFFFFFFFFFFFFFF43ULL;

#include <cstdint>

inline constexpr std::uint64_t SQRT_2  = 0x3FF6A09E667F3BCDULL; // ~1.4142
inline constexpr std::uint64_t SQRT_3  = 0x3FFBB67AE8584CAAULL; // ~1.7321
inline constexpr std::uint64_t SQRT_5  = 0x4001E3779B97F4A8ULL; // ~2.2361
inline constexpr std::uint64_t SQRT_6  = 0x4003988E1409212EULL; // ~2.4495
inline constexpr std::uint64_t SQRT_7  = 0x40052A7FA9D2F8EAULL; // ~2.6458
inline constexpr std::uint64_t SQRT_8  = 0x4006A09E667F3BCDULL; // ~2.8284
inline constexpr std::uint64_t SQRT_10 = 0x40094C583ADA5B53ULL; // ~3.1623
inline constexpr std::uint64_t SQRT_11 = 0x400A887293FD6F34ULL; // ~3.3166
inline constexpr std::uint64_t SQRT_12 = 0x400BB67AE8584CAAULL; // ~3.4641
inline constexpr std::uint64_t SQRT_13 = 0x400CD82B446159F3ULL; // ~3.6056
inline constexpr std::uint64_t SQRT_14 = 0x400DEEEA11683F49ULL; // ~3.7417
inline constexpr std::uint64_t SQRT_15 = 0x400EFBDEB14F4EDAULL; // ~3.8730

static constexpr uint64_t J_MIX[4] = {H2_C1, H2_C2, H2_C3, H2_C4};
static constexpr uint64_t J_MIX2[4] = {SQRT_2, SQRT_5, SQRT_7, SQRT_13};


static constexpr unsigned char Rcon[256] = {
0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x08
};


inline constexpr std::array<uint64_t, 256> pregenerate_mixtable(uint64_t in) {
    std::array<uint64_t, 256> table{};
    for(int i = 0; i < 256; i++) {
        uint64_t base = in * SQRT_7 + i;
        uint64_t sum = static_cast<uint64_t>(sbox[i]) + Rcon[i] + J_MIX[i%4] + J_MIX2[i%4];
        uint64_t val = (sum * 0x94D049BB133111EBULL) ^ (sum * SQRT_3);
        table[i] = base ^ val;
    }
    return table;
}

inline constexpr std::array<uint64_t, 256> MIXTABLE = pregenerate_mixtable(SQRT_3);

inline uint64_t mixtable_64(uint64_t x) {
    return uint64_t(MIXTABLE[ x        & 0xff])       ^
          (uint64_t(MIXTABLE[(x >>  8) & 0xff])) ^
          (uint64_t(MIXTABLE[(x >> 16) & 0xff])) ^
          (uint64_t(MIXTABLE[(x >> 24) & 0xff])) ^
          (uint64_t(MIXTABLE[(x >> 32) & 0xff])) ^
          (uint64_t(MIXTABLE[(x >> 40) & 0xff])) ^
          (uint64_t(MIXTABLE[(x >> 48) & 0xff])) ^
          (uint64_t(MIXTABLE[(x >> 56)        ]));
}

inline uint64_t arx_l(const uint64_t a,const uint64_t b,const uint64_t c, int d) {
    return std::rotl((a + b) ^ c, d);
}

inline uint64_t arx_r(const uint64_t a,const uint64_t b,const uint64_t c, int d) {
    return std::rotr((a + b) ^ c, d);
}

inline constexpr std::uint64_t choose(std::uint64_t x, std::uint64_t y, std::uint64_t z) {
    return (x & y) ^ (~x & z);
}

inline constexpr uint64_t Maj(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

inline constexpr uint64_t Parity(uint64_t x, uint64_t y, uint64_t z)
{
    return x ^ y ^ z;
}

inline constexpr uint64_t chi(
    uint64_t x,
    uint64_t y,
    uint64_t z)
{
    return x ^ ((~y) & z);
}

inline void blake_mix(uint256& u) {
    u.a += u.b; u.d ^= u.a; u.d = std::rotr(u.d, 32);
    u.c += u.d; u.b ^= u.c; u.b = std::rotr(u.b, 24);
    u.a += u.b; u.d ^= u.a; u.d = std::rotr(u.d, 16);
    u.c += u.d; u.b ^= u.c; u.b = std::rotr(u.b, 63);
}

inline void blake_mix_2(uint256& u) {
    u.a += u.b; u.d ^= u.a; u.d = std::rotl(u.d, 63);
    u.c += u.d; u.b ^= u.c; u.b = std::rotl(u.b, 16);
    u.a += u.b; u.d ^= u.a; u.d = std::rotl(u.d, 24);
    u.c += u.d; u.b ^= u.c; u.b = std::rotl(u.b, 32);
}

inline void mixNmix(uint256& u) {
    blake_mix(u);
    uint64_t ta = splitmix(u.d) ^ H2_C1;
    uint64_t tb = splitmix(u.c) ^ H2_C2;
    uint64_t tc = splitmix(u.b) ^ H2_C3;
    uint64_t td = splitmix(u.a) ^ H2_C4;

    u.a ^= ta;
    u.b ^= tb;
    u.c ^= tc;
    u.d ^= td;
    blake_mix_2(u);
}

inline void h2_process_tail(uint256& u, const uint8_t* p, size_t n) {
    for (size_t i = 0; i < n;) {

        if(i+8<=n) {
            uint64_t w;
            std::memcpy(&w, p+i, sizeof(w));
            const uint64_t wx = std::rotr(w, 32) ^ std::rotl(w, 49);
            for(int j=0;j<4;j++) {
                const uint64_t x = Maj(w, w, mixtable_64(wx));
                const uint64_t y = choose(wx, sbox64(x), mixtable_64(x));
                const uint64_t z = Parity(w, x, y);

                u.v[j] += x ^ y ^ z;

                u.v[j] *= H2_C1;

                u.v[j] = std::rotl(u.v[j], 27) ^ std::rotl(u.v[j], 53);
            }
            blake_mix(u);
            i+=8;
        } else {
            const uint64_t y = (p[i] ^ SQRT_13) * 13;
            for(uint64_t j=0;j<4;j++) {
                const uint64_t x = Maj(y, sbox64(y), mixtable_64(y));
                const uint64_t w = arx_l(u.v[j], x, H2_C3, 29);
                const uint64_t z = Parity(w, x, y);

                u.v[j] ^= std::rotl(z, 61) + J_MIX2[j];
                u.v[j] ^= (z + w + x + y);
            }
            blake_mix(u);
            ++i;
        }
    }
    mixNmix(u);
}

inline void h2_process_block(uint256& u, const uint8_t* p)
{
    uint64_t w[4];
    std::memcpy(&w, p, sizeof(w));

    uint64_t wx = std::rotl(w[0], 47) ^ std::rotr(w[1], 61) ^ std::rotl(w[2], 31) ^ std::rotr(w[3], 27); 

    for(int i=0;i<4;i++) {
        const uint64_t m = mum_mix(w[i]^wx, H2_C1);
        const uint64_t c = choose(sbox64(w[i]), w[i]*H2_C1, w[i]*SQRT_7) * SQRT_5;
        const uint64_t t = mixtable_64(w[i]);
        const uint64_t r = std::rotl(w[i], i+13) ^ std::rotr(w[i], i+17);

        u.v[i] ^= std::rotl(u.v[i] ^ m ^ r, (i+8)) ^ c ^ t ^ std::rotl(u.v[((i+1)%4)], 13);
        u.v[i] ^= (mum_mix(w[i], H2_C3)) ^ (mum_mix(wx, H2_C1));
    }
    blake_mix(u);
    blake_mix(u);
}

inline void h2_end(uint256& u) {
    mixNmix(u);
    u.a ^= std::rotl(u.b, 13);
    u.c ^= std::rotl(u.d, 29);
    u.b ^= mum_mix(u.c, SQRT_7);
    u.d ^= mum_mix(u.a, SQRT_11);

    const uint64_t t0 = u.a * SQRT_11;
    const uint64_t t1 = u.b * SQRT_7;
    const uint64_t t2 = u.c * SQRT_5;
    const uint64_t t3 = u.d * SQRT_2;
    u.a ^= t3; u.b ^= t2; u.c ^= t1; u.d ^= t0;
    for(int j=0;j<4;j++) {
        const uint64_t x = arx_l(u.v[(j+1)%4], 13, u.v[j], 23) + arx_r(u.v[j], 13, u.v[(j+1)%4], 61);
        u.v[j] = x;
        for(int i=3;i>=0;i--) {
            u.v[j] += u.v[i];
            u.v[j] = arx_l(u.v[j], H2_C5, u.v[i], (i*j)+13);
        }
        u.v[j] ^= mixtable_64(x);
    }

    mixNmix(u);
}

inline void h2_round(uint256& u, std::span<const uint8_t> in)
{
    size_t i = 0;

    for(; i + 32 <= in.size(); i += 32) {
        if (i + 256 < in.size()) __builtin_prefetch(in.data() + i + 256, 0, 0);
        h2_process_block(u, in.data() + i);
    }

    if (i < in.size())
        h2_process_tail(u, in.data() + i, in.size() - i);


    h2_end(u);
}

inline void h2_body(uint256& hash, const uint64_t size) {
    blake_mix(hash);
    for(int k=0;k<16;k++) {
        uint64_t t[4];
        t[0] = mum_mix(hash.v[0] ^ hash.v[1], H2_C1) + size;
        t[1] = mum_mix(hash.v[1] ^ hash.v[2], H2_C3) + size;
        t[2] = mum_mix(hash.v[2] ^ hash.v[3], H2_C4) + size;
        t[3] = mum_mix(hash.v[3] ^ hash.v[0], H2_C2) + size;
        for(int i=0;i<4;i++) hash.v[i] ^= t[(i+1)%4];
        mixNmix(hash);

        for(int i=0; i<4; i++) {hash.v[i] ^= t[i];hash.v[i] = mum_mix(hash.v[i], SQRT_7);}
        for(int i=0; i<4; i++) {hash.v[i] = mum_mix(hash.v[i], SQRT_3);}

        uint64_t ta = splitmix(hash.d) ^ SQRT_7;
        uint64_t tb = splitmix(hash.c) ^ SQRT_13;
        uint64_t tc = splitmix(hash.b) ^ SQRT_2;
        uint64_t td = splitmix(hash.a) ^ SQRT_5;

        mixNmix(hash);

        hash.a ^= ta;
        hash.b ^= tb;
        hash.c ^= tc;
        hash.d ^= td;

        mixNmix(hash);
    }
}

uint256 h2_hash(const std::span<const uint8_t> in) {
    uint256 hash{H2_C1, H2_C2, H2_C3, H2_C4};
    const uint64_t size = in.size();

    h2_round(hash, in);

    h2_body(hash, size);
    return hash;
}

class H2HashStreaming {
public:
    void update(std::span<const uint8_t> data)
    {
        size+=data.size();
        if (buffer_size != 0) {
            const size_t n =
                std::min<size_t>(32 - buffer_size, data.size());

            std::memcpy(
                buffer + buffer_size,
                data.data(),
                n
            );

            buffer_size += n;
            data = data.subspan(n);

            if (buffer_size == 32) {
                h2_process_block(state, buffer);
                buffer_size = 0;
            }
        }

        while (data.size() >= 32) {
            h2_process_block(state, data.data());
            data = data.subspan(32);
        }

        if (!data.empty()) {
            std::memcpy(buffer, data.data(), data.size());
            buffer_size = data.size();
        }
    }

    uint256 finalize()
    {
        uint256 hash = state;

        if (buffer_size != 0) {
            h2_process_tail(hash, buffer, buffer_size);
        }

        h2_end(hash);

        h2_body(hash, size);
        return hash;
    }

    void reset()
    {
        state = {H2_C1, H2_C2, H2_C3, H2_C4};
        buffer_size = 0;
        size = 0;
    }

private:
    uint256 state{H2_C1, H2_C2, H2_C3, H2_C4};

    uint8_t buffer[32]{};
    size_t buffer_size = 0;
    uint64_t size=0;
};

void H2_SMHasher(const void* key, int len, uint32_t seed, void* out)
{
    auto input = std::span<const uint8_t>(
        static_cast<const uint8_t*>(key),
        static_cast<size_t>(len)
    );

    uint256 hash{H2_C1^seed, H2_C2^seed, H2_C3^seed, H2_C4^seed};
    const uint64_t size = input.size();

    h2_round(hash, input);

    h2_body(hash, size);
    std::memcpy(static_cast<uint8_t*>(out) +  0, &hash.a, 8);
    std::memcpy(static_cast<uint8_t*>(out) +  8, &hash.b, 8);
    std::memcpy(static_cast<uint8_t*>(out) + 16, &hash.c, 8);
    std::memcpy(static_cast<uint8_t*>(out) + 24, &hash.d, 8);
}

#endif
