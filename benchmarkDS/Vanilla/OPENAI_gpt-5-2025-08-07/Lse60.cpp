#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>

std::string allocate_and_describe(long long size) {
    if (size < 0) size = 0;
    std::vector<unsigned char> buffer(static_cast<size_t>(size), 0); // zero-initialized
    std::ostringstream oss;
    oss << std::uppercase << std::hex;
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (i > 0) oss << ' ';
        oss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buffer[i]);
    }
    return oss.str();
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        long long v = std::atoll(argv[1]);
        std::cout << allocate_and_describe(v) << std::endl;
    } else {
        long long tests[5] = {0, 1, 3, 8, 16};
        for (int i = 0; i < 5; ++i) {
            std::cout << allocate_and_describe(tests[i]) << std::endl;
        }
    }
    return 0;
}