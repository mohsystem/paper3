#include <iostream>
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>

unsigned char* copy_bytes(unsigned char* dest, const unsigned char* src, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        dest[i] = src[i];
    }
    return dest;
}

int main() {
    struct TestCase { std::string src; std::size_t n; std::size_t dest_size; };
    std::vector<TestCase> tests = {
        {"Hello, World!", 13, 20},
        {"Hello, World!", 5, 5},
        {"Data", 4, 10},
        {"ByteCopy", 0, 10},
        {"Sample", 6, 6},
    };

    for (std::size_t i = 0; i < tests.size(); ++i) {
        const auto& t = tests[i];
        std::vector<unsigned char> dest(t.dest_size, 0);
        copy_bytes(dest.data(),
                   reinterpret_cast<const unsigned char*>(t.src.data()),
                   t.n);
        std::cout << "Test " << (i + 1) << ": \"";
        std::cout.write(reinterpret_cast<const char*>(dest.data()), static_cast<std::streamsize>(t.n));
        std::cout << "\"\n";
    }

    return 0;
}