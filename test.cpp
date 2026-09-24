#include "h2.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>
#include <span>

static bool equal_hash(const uint256& a, const uint256& b)
{
    return (a.a ^ b.a) == 0 &&
           (a.b ^ b.b) == 0 &&
           (a.c ^ b.c) == 0 &&
           (a.d ^ b.d) == 0;
}

static void print_hash(const uint256& h)
{
    printf("%016llx%016llx%016llx%016llx",
           (unsigned long long)h.d,
           (unsigned long long)h.c,
           (unsigned long long)h.b,
           (unsigned long long)h.a);
}

template <size_t BYTES>
void test_size(uint64_t count)
{
    std::array<uint8_t, BYTES> input{};

    std::mt19937_64 rng(0x123456789ABCDEF0ULL + BYTES);

    uint64_t collisions = 0;

    printf("\nTesting %zu-bit inputs: %llu inputs\n",
           BYTES * 8,
           (unsigned long long)count);

    for (uint64_t i = 0; i < count; ++i) {

        // Generate deterministic random input.
        for (size_t j = 0; j < BYTES; j += 8) {
            uint64_t x = rng();

            size_t n = (BYTES - j < 8)
                     ? BYTES - j
                     : 8;

            std::memcpy(input.data() + j, &x, n);
        }

        uint256 h = h2_hash(
            std::span<const uint8_t>(
                input.data(),
                input.size()
            )
        );

        /*
         * Exact collision testing requires remembering previous hashes.
         *
         * This version instead performs a differential self-test:
         * flip one bit and check whether the resulting hash is identical.
         */
        for (size_t bit = 0; bit < BYTES * 8; ++bit) {

            size_t byte = bit / 8;
            uint8_t mask = uint8_t(1u << (bit & 7));

            input[byte] ^= mask;

            uint256 h2 = h2_hash(
                std::span<const uint8_t>(
                    input.data(),
                    input.size()
                )
            );

            if (equal_hash(h, h2)) {
                ++collisions;

                printf("\nCOLLISION!\n");
                printf("size: %zu bits\n", BYTES * 8);
                printf("input #%llu\n",
                       (unsigned long long)i);
                printf("flipped bit: %zu\n", bit);
                printf("hash: ");
                print_hash(h);
                printf("\n");
            }

            input[byte] ^= mask;
        }

        if ((i + 1) % 1'000'000 == 0) {
            printf("\r%llu / %llu",
                   (unsigned long long)(i + 1),
                   (unsigned long long)count);
            fflush(stdout);
        }
    }

    printf("\nCollisions: %llu\n",
           (unsigned long long)collisions);
}

int main()
{
    constexpr uint64_t N = 1'000'000;

    test_size<4>(N);      // 32 bits
    test_size<8>(N);      // 64 bits
    test_size<16>(N);     // 128 bits
    test_size<32>(N);     // 256 bits
    test_size<64>(N);     // 512 bits
    test_size<128>(N);    // 1024 bits

    return 0;
}