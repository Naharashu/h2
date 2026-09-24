#include "h2.hpp"
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>

int main(int argc, char* argv[]) {
    if(argc<2) {
        std::cout << "Usage: h2sum [file]\n";
        return 1;
    }

    const std::string arg = argv[1];

    if(arg=="-v") {
        std::cout << "h2sum 1.0.0 with H2 hash 2.0\n";
        return 0;
    }

    std::ifstream f(arg, std::ios::binary | std::ios::in);

    if(!f.is_open()) {
        std::cerr << "h2sum: file " << arg << " doesn't exist\n";
        return 1;
    }

    constexpr size_t CHUNK = 1024 * 1024;
    std::vector<uint8_t> chunk(CHUNK);

    H2HashStreaming hasher;

    while (f) {
        f.read(reinterpret_cast<char*>(chunk.data()), CHUNK);
        size_t bytes_read = f.gcount();
        if (bytes_read == 0) break;
        hasher.update(std::span<const uint8_t>(chunk.data(), bytes_read));
    }

    uint256 hash = hasher.finalize();
    std::cout << std::hex << hash.a << hash.b << hash.c << hash.d << '\n';
    return 0;
}