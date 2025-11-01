#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <limits>

int getValueFromArray(const std::vector<int>& arr, std::size_t size, std::size_t index) {
    if (size > arr.size()) {
        throw std::runtime_error("Invalid size");
    }
    if (index >= size) {
        throw std::out_of_range("Index out of bounds");
    }
    return arr[index];
}

int main() {
    std::vector<int> example{10, 20, 30, 40, 50};
    std::size_t size = example.size();

    std::string line;
    if (!std::getline(std::cin, line)) {
        // No input provided: run 5 test cases
        for (std::size_t i = 0; i < size; ++i) {
            try {
                int value = getValueFromArray(example, size, i);
                std::cout << "Test " << (i + 1) << ": " << value << "\n";
            } catch (...) {
                std::cout << "Test " << (i + 1) << ": error\n";
            }
        }
        return 0;
    }

    // Trim
    std::size_t start = line.find_first_not_of(" \t\r\n");
    std::size_t end = line.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) {
        // Empty input: run tests
        for (std::size_t i = 0; i < size; ++i) {
            try {
                int value = getValueFromArray(example, size, i);
                std::cout << "Test " << (i + 1) << ": " << value << "\n";
            } catch (...) {
                std::cout << "Test " << (i + 1) << ": error\n";
            }
        }
        return 0;
    }
    std::string trimmed = line.substr(start, end - start + 1);

    // Parse integer safely
    std::istringstream iss(trimmed);
    long long idx_ll;
    if (!(iss >> idx_ll) || !(iss.eof())) {
        return 1;
    }
    if (idx_ll < 0 || static_cast<std::size_t>(idx_ll) >= size) {
        return 1;
    }

    try {
        int value = getValueFromArray(example, size, static_cast<std::size_t>(idx_ll));
        return (value & 0xFF);
    } catch (...) {
        return 1;
    }
}