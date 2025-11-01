#include <iostream>
#include <array>
#include <string>
#include <stdexcept>

static const std::array<int, 10> ID_SEQUENCE = {101, 203, 304, 405, 506, 607, 708, 809, 910, 1001};

int getIdAtIndex(int index) {
    if (index < 0 || static_cast<size_t>(index) >= ID_SEQUENCE.size()) {
        throw std::out_of_range("Invalid index.");
    }
    return ID_SEQUENCE[static_cast<size_t>(index)];
}

int main() {
    try {
        std::cout << "Enter an index (0.." << (ID_SEQUENCE.size() - 1) << "): ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "I/O error.\n";
        } else {
            try {
                size_t pos = 0;
                int idx = std::stoi(line, &pos, 10);
                if (pos != line.size()) {
                    std::cout << "Invalid input.\n";
                } else {
                    int value = getIdAtIndex(idx);
                    std::cout << "Value at index " << idx << ": " << value << "\n";
                }
            } catch (const std::invalid_argument&) {
                std::cout << "Invalid input.\n";
            } catch (const std::out_of_range&) {
                std::cout << "Invalid index.\n";
            }
        }
    } catch (...) {
        std::cout << "I/O error.\n";
    }

    // 5 Test cases
    std::cout << "Running test cases:\n";
    int tests[5] = {0, 3, 9, -1, 10};
    for (int idx : tests) {
        try {
            int val = getIdAtIndex(idx);
            std::cout << "Index " << idx << " => " << val << "\n";
        } catch (...) {
            std::cout << "Index " << idx << " => error\n";
        }
    }
    return 0;
}