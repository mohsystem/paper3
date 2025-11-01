#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <stdexcept>

static const long long OFFSET = 1000LL;

long long addOffset(long long value) {
    if (value > LLONG_MAX - OFFSET) {
        throw std::overflow_error("Overflow when adding offset");
    }
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    std::vector<long long> tests = {0LL, 1LL, 999LL, -1000LL, 1234567890LL};
    for (auto t : tests) {
        try {
            long long result = addOffset(t);
            std::cout << "Test input: " << t << " -> " << result << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Test input: " << t << " -> Error: " << e.what() << "\n";
        }
    }

    if (argc > 1) {
        try {
            std::string s(argv[1]);
            size_t idx = 0;
            long long value = std::stoll(s, &idx, 10);
            if (idx != s.size()) {
                throw std::invalid_argument("Trailing characters");
            }
            long long result = addOffset(value);
            std::cout << "Command-line input: " << value << " -> " << result << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Invalid input: " << e.what() << "\n";
        }
    }
    return 0;
}