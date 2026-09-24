# H2 Hashing Function

H2 is simple 256-bit hash function. It's not very fast but get's job done. `h2.hpp` is header-only C++20 implementation. H2 it is direct successor to mine old [H1](https://github.com/Naharashu/HASH1) algorithm. Hash passed basic tests on avalanche and collision.

# Perfomance 

Everything tested on i3-3120m x64 cpu with -O3 and -march=native

### Incremental hashing with 1MB block size:

| Compiler | 64B Input | 1024B input | 64KB input | 16 MB input | Avarage |
| -------- | -------- | -------- | -------- | -------- | -------- |
| Clang 22.1.8 | 541.42 MB/s | **947.73 MB/s** | **997.44 MB/s** | **989.62 MB/s** | 869,05 MB/s |
| GCC 16.2.1   | **626.92 MB/s** | 932.10 MB/s | 986.17 MB/s | 981.72 MB/s | **881.72 MB/s** |


### Sequential hashing:

| Compiler | 64B Input | 1024B input | 64KB input | 16 MB input | Avarage |
| -------- | -------- | -------- | -------- | -------- | -------- |
| Clang 22.1.8 | 541.42 MB/s | **947.73 MB/s** | **997.44 MB/s** | **989.62 MB/s** | 869,05 MB/s |
| GCC 16.2.1   | **626.92 MB/s** | 932.10 MB/s | 986.17 MB/s | 981.72 MB/s | **881.72 MB/s** |