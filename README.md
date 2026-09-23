# H2 Hashing Function

H2 is simple 256-bit hash function. It's not very fast but get's job done. `h2.hpp` is header-only C++20 implementation. H2 it is direct successor to mine old [H1](https://github.com/Naharashu/HASH1) algorithm. Hash passed basic tests on avalanche and collision.

# Perfomance 

Everything tested on i3-3120m x64 cpu with -O3 and -march=native


| Compiler | 64B Input | 1024B input | 65KB input | Avarage |
| -------- | -------- | -------- | -------- | -------- |
| Clang 22.1.8 | 191.49 MB/s  | 213.68 MB/s | **215.62 MB/s** | **206.93 MB/s**  |
| GCC 16.2.1 |  191.49 MB/s | **217.18 MB/s**  | 204.71 MB/s  | 204.46 MB/s  |
