#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <limits>

static const std::size_t MAX_SIZE = 1048576; // 1 MiB cap

std::string allocate_and_dump_buffer(std::size_t size) {
    if (size > MAX_SIZE) {
        throw std::invalid_argument("Invalid size");
    }
    if (size == 0) {
        return std::string();
    }
    std::vector<unsigned char> buf(size, 0); // zero-initialized
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << std::setfill('0');
    for (std::size_t i = 0; i < buf.size(); ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(buf[i]);
        if (i + 1 < buf.size()) oss << ' ';
    }
    return oss.str();
}

int main(int argc, char* argv[]) {
    try {
        if (argc >= 2) {
            // Parse as unsigned long long then range-check
            std::string arg = argv[1];
            std::size_t idx = 0;
            unsigned long long parsed = 0ULL;
            try {
                parsed = std::stoull(arg, &idx, 10);
            } catch (...) {
                std::cerr << "Error: invalid integer size\n";
                return 1;
            }
            if (idx != arg.size() || parsed > MAX_SIZE) {
                std::cerr << "Error: size must be between 0 and " << MAX_SIZE << "\n";
                return 1;
            }
            std::string out = allocate_and_dump_buffer(static_cast<std::size_t>(parsed));
            if (!out.empty()) {
                std::cout << out << "\n";
            } else {
                std::cout << "\n";
            }
        } else {
            std::size_t tests[5] = {0, 1, 8, 16, 64};
            for (std::size_t t : tests) {
                std::string out = allocate_and_dump_buffer(t);
                if (!out.empty()) {
                    std::cout << out << "\n";
                } else {
                    std::cout << "\n";
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}