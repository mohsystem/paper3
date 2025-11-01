#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <stdexcept>

static const std::size_t MAX_BUFFER_SIZE = 1'048'576; // 1 MiB

std::string secureDumpBuffer(std::size_t size) {
    if (size > MAX_BUFFER_SIZE) {
        throw std::invalid_argument("Size too large");
    }
    if (size == 0) {
        return std::string();
    }

    std::vector<unsigned char> buffer(size);
    for (std::size_t i = 0; i < size; ++i) {
        buffer[i] = static_cast<unsigned char>(i & 0xFF); // deterministic pattern
    }

    std::string result;
    // Each byte contributes "XX " or "XX\n" => 3 chars per byte
    if (size <= (std::numeric_limits<std::size_t>::max)() / 3) {
        result.reserve(size * 3);
    }

    static const char HEX[] = "0123456789ABCDEF";
    for (std::size_t i = 0; i < size; ++i) {
        unsigned char v = buffer[i];
        result.push_back(HEX[(v >> 4) & 0x0F]);
        result.push_back(HEX[v & 0x0F]);
        if ((i & 15) == 15) {
            result.push_back('\n');
        } else {
            result.push_back(' ');
        }
    }
    if ((size & 15) != 0 && !result.empty() && result.back() == ' ') {
        result.back() = '\n';
    }
    return result;
}

static void runSingle(std::size_t size) {
    try {
        std::string out = secureDumpBuffer(size);
        if (!out.empty()) {
            std::cout << out;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        std::exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        try {
            std::size_t val = 0;
            std::string arg(argv[1]);
            if (arg.empty()) {
                throw std::invalid_argument("empty argument");
            }
            std::size_t pos = 0;
            unsigned long long tmp = std::stoull(arg, &pos, 10);
            if (pos != arg.size()) {
                throw std::invalid_argument("invalid numeric format");
            }
            if (tmp > (std::numeric_limits<std::size_t>::max)()) {
                throw std::out_of_range("value too large");
            }
            val = static_cast<std::size_t>(tmp);
            runSingle(val);
            return 0;
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << '\n';
            return 1;
        }
    }

    // 5 test cases when no argument is provided
    std::size_t tests[5] = {0, 1, 16, 31, 64};
    for (std::size_t t : tests) {
        std::cout << "=== size " << t << " ===\n";
        runSingle(t);
    }
    return 0;
}