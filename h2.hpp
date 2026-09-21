#ifndef H2_HASH_HPP
#define H2_HASH_HPP

/*
    Header-only library of H2 hash
    H2 hash algorithm by Naharashu (me)
    Apache 2.0 License
    Work in progress
*/

#include <cstdint>
#include <vector>
struct uint256 {
    std::uint64_t a, b, c, d;
};

inline uint64_t h2_hepler_sum(uint64_t start, uint64_t end, const uint64_t& xor_with, const std::vector<uint8_t>& in) {
    uint64_t sum = 0;
    for(uint64_t i=0;i<end;i++) {
        sum += in[i];
    }
    sum ^= xor_with;
    return sum;
}

inline uint64_t h2_hepler_xor(uint64_t start, uint64_t end, const uint64_t& add_with, const std::vector<uint8_t>& in) {
    uint64_t sum = 0;
    for(uint64_t i=start;i<end;i++) {
        sum ^= in[i];
    }
    sum += add_with;
    return sum;
}

inline uint64_t arx_l(const uint64_t& a,const uint64_t& b,const uint64_t& c, const int& d) {
    return std::rotl((a + b) ^ c, d);
}

inline uint64_t arx_r(const uint64_t& a,const uint64_t& b,const uint64_t& c, const int& d) {
    return std::rotr((a + b) ^ c, d);
}

inline uint64_t h2_hepler_arx_l(uint64_t start, uint64_t end, const uint64_t& add_with, const std::vector<uint8_t>& in) {
    uint64_t sum = 0;
    for(uint64_t i=start;i-1<end;i++) {
        sum += arx_l(in[i], in[i+1], add_with, 41);
    }
    sum += add_with;
    return sum;
}


uint256 h2_hash(const std::vector<uint8_t>& in) {
    uint256 hash = uint256{0,0,0,0};
    const uint64_t size = in.size();

    hash.a = h2_hepler_sum(0, (size%32), 0x9e3779b97f4a7c15, in);

    hash.b = arx_l(hash.a, 31, 0xbb67ae8584caa73b, 13);

    hash.c = h2_hepler_xor(0, size, 0xb7e151628aed2a6a, in);

    hash.d = h2_hepler_arx_l(0, size/2, 0xab1c5ed5da6d8118, in);

    hash.d = arx_r(hash.c, hash.a, hash.d, 33);

    hash.a *= UINT64_MAX + hash.c;

    hash.b += hash.a ^ 0x6a09e667f3bcc908;

    hash.d = arx_l(hash.d, 0x1fffffffffffffff, hash.a, 53);

    return hash;
}

#endif