# H2 Hashing Function

H2 is simple 256-bit hash function. It's not very fast but get's job done. `h2.hpp` is header-only C++20 implementation.

# Perfomance 

Everything tested on i3-3120m x64 cpu with -O3 and -march=native


| Compiler | 64B Input | 1024B input | 65KB input | Avarage |
| -------- | -------- | -------- | -------- | -------- |
| Clang 22.1.8 | 65.19 MB/s  | 65.12 MB/s | 65.24 MB/s | 65.18 MB/s  |
| GCC 16.2.1 |  73.87 MB/s | 73.86 MB/s  | 73.98 MB/s  | 73.90 MB/s  |
