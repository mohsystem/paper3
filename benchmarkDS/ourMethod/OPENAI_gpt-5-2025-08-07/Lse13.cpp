#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

static const std::vector<int> ID_SEQUENCE = {7, 13, 21, 34, 55, 89, 144, 233, 377, 610};

int getValueAtIndex(std::size_t index) {
    if (index >= ID_SEQUENCE.size()) {
        throw std::out_of_range("invalid index");
    }
    return ID_SEQUENCE[index];
}

static std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

int main() {
    std::cout << "Enter an index (0.." << (ID_SEQUENCE.size() - 1) << "): ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cerr << "Error: no input.\n";
    } else {
        std::string s = trim(line);
        if (s.empty() || s.size() > 64) {
            std::cerr << "Error: invalid input.\n";
        } else {
            try {
                std::size_t pos = 0;
                long long val = std::stoll(s, &pos, 10);
                if (pos != s.size()) {
                    std::cerr << "Error: invalid input.\n";
                } else if (val < 0 || static_cast<unsigned long long>(val) >= ID_SEQUENCE.size()) {
                    std::cerr << "Error: invalid index.\n";
                } else {
                    int v = getValueAtIndex(static_cast<std::size_t>(val));
                    std::cout << "Value at index " << val << " = " << v << "\n";
                }
            } catch (...) {
                std::cerr << "Error: invalid input.\n";
            }
        }
    }

    // 5 test cases
    const std::vector<std::size_t> tests = {0, ID_SEQUENCE.size() - 1, 2, 5, 7};
    for (std::size_t t : tests) {
        try {
            int v = getValueAtIndex(t);
            std::cout << "Test index " << t << " -> " << v << "\n";
        } catch (...) {
            std::cout << "Test index " << t << " -> error\n";
        }
    }
    return 0;
}