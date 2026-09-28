# H2 Hashing Function

H2 is simple 256-bit hash function. It's not very fast but get's job done. `h2.hpp` is header-only C++20 implementation. H2 it is direct successor to mine old [H1](https://github.com/Naharashu/HASH1) algorithm. Hash passed basic tests on avalanche and collision.

# Perfomance 

Everything tested on i3-3120m x64 cpu with -O3 and -march=native

### Incremental hashing with 1MB chunk size:

| Compiler     | 64B Input    | 1024B Input  | 64KB Input   | 64MB Input   | Average      |
|--------------|--------------|--------------|--------------|--------------|--------------|
| Clang 22.1.8 | 40.97 MB/s   | 329.85 MB/s  | 405.63 MB/s  | 509.52 MB/s  | 321.49 MB/s  |
| GCC 16.2.1   | 40.16 MB/s   | 316.07 MB/s  | 407.86 MB/s  | 513.91 MB/s  | 319.50 MB/s  |

### Incremental hashing with 256B chunk size:

| Compiler     | 64B Input   | 1024B Input  | 64KB Input   | 64MB Input   | Average      |
|--------------|-------------|--------------|--------------|--------------|--------------|
| Clang 22.1.8 | 40.48 MB/s  | 328.73 MB/s  | 453.40 MB/s  | 507.03 MB/s  | 332.41 MB/s  |
| GCC 16.2.1   | 37.37 MB/s  | 315.68 MB/s  | 425.18 MB/s  | 512.49 MB/s  | 322.68 MB/s  |

### Sequential hashing:

| Compiler     | 64B          | 1024B        | 64KB         | 64MB         |
|--------------|--------------|--------------|--------------|--------------|
| GCC 16.2.1   | 46.86 MB/s   | 310.28 MB/s  | 501.44 MB/s  | 503.97 MB/s  |
| Clang 22.1.8 | 50.93 MB/s   | 324.01 MB/s  | 504.40 MB/s  | 505.91 MB/s  |