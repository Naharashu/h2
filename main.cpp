
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <random>
#include <algorithm>

#include "h2.hpp"

using namespace std;
using namespace std::chrono;

// Prevent the compiler from removing the hashing work.
volatile uint64_t benchmark_sink = 0;

static void benchmark_size(size_t size, int iterations)
{
    vector<uint8_t> data(size);

    // Deterministic pseudo-random input.
    uint64_t seed = 0x123456789abcdef0ULL;

    for (size_t i = 0; i < size; ++i) {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;

        data[i] = static_cast<uint8_t>(seed);
    }

    // Warm-up
    for (int i = 0; i < 3; ++i) {
        uint256 h = h2_hash(data);
        benchmark_sink ^= h.a ^ h.b ^ h.c ^ h.d;
    }

    auto start = steady_clock::now();

    for (int i = 0; i < iterations; ++i) {
        uint256 h = h2_hash(data);

        benchmark_sink ^= h.a;
        benchmark_sink ^= h.b;
        benchmark_sink ^= h.c;
        benchmark_sink ^= h.d;
    }

    auto end = steady_clock::now();

    double seconds =
        duration<double>(end - start).count();

    double total_bytes =
        static_cast<double>(size) * iterations;

    double mib =
        total_bytes / (1024.0 * 1024.0);

    double mib_per_sec =
        mib / seconds;

    double ns_per_hash =
        (seconds * 1e9) / iterations;

    printf(
        "%10zu B | %8.2f MiB/s | %10.2f ns/hash | %d hashes\n",
        size,
        mib_per_sec,
        ns_per_hash,
        iterations
    );
}

int main()
{
    printf("H2 HASH PERFORMANCE TEST\n");
    printf("========================\n\n");

#if defined(__clang__)
    printf("Compiler: Clang %s\n", __clang_version__);
#elif defined(__GNUC__)
    printf("Compiler: GCC %s\n", __VERSION__);
#else
    printf("Compiler: unknown\n");
#endif

#if defined(__x86_64__)
    printf("Architecture: x86-64\n");
#elif defined(__aarch64__)
    printf("Architecture: AArch64\n");
#else
    printf("Architecture: other\n");
#endif

    printf("\n");
    printf("%10s | %13s | %15s | %s\n",
           "Input", "Throughput", "Latency", "Iterations");
    printf("-----------+---------------+-----------------+-----------\n");

    // Small messages: measure fixed overhead.
    benchmark_size(0,       1000000);
    benchmark_size(1,       1000000);
    benchmark_size(8,       1000000);
    benchmark_size(32,      1000000);
    benchmark_size(64,      1000000);
    benchmark_size(128,      500000);
    benchmark_size(256,      500000);
    benchmark_size(1024,     100000);

    // Larger messages: measure actual hashing throughput.
    benchmark_size(4 * 1024,      50000);
    benchmark_size(16 * 1024,     20000);
    benchmark_size(64 * 1024,      5000);
    benchmark_size(1024 * 1024,     500);
    benchmark_size(16 * 1024 * 1024, 30);
    benchmark_size(64 * 1024 * 1024, 10);

    printf("\nSink: %016llx\n",
           static_cast<unsigned long long>(benchmark_sink));

    return 0;
}
