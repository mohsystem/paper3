#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

static const int id_sequence[] = {101, 203, 307, 409, 512, 678, 789, 890, 901, 1024};
static constexpr std::size_t ID_LEN = sizeof(id_sequence) / sizeof(id_sequence[0]);

int getIdAtIndex(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= ID_LEN) {
        throw std::out_of_range("Index out of bounds");
    }
    return id_sequence[index];
}

int main() {
    std::cout << "Enter index (0-" << (ID_LEN - 1) << "): ";
    std::string line;
    if (std::getline(std::cin, line)) {
        try {
            std::size_t pos = 0;
            long long val = std::stoll(line, &pos, 10);
            if (pos != line.size()) throw std::invalid_argument("Trailing characters");
            if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) {
                throw std::out_of_range("Value out of int range");
            }
            int idx = static_cast<int>(val);
            int value = getIdAtIndex(idx);
            std::cout << "Value at index " << idx << " is: " << value << "\n";
        } catch (const std::invalid_argument&) {
            std::cout << "Invalid input: not a valid integer.\n";
        } catch (const std::out_of_range&) {
            std::cout << "Invalid index: must be between 0 and " << (ID_LEN - 1) << ".\n";
        } catch (...) {
            std::cout << "An unexpected error occurred.\n";
        }
    } else {
        std::cout << "No input received.\n";
    }

    // Five test cases
    std::cout << "Running 5 test cases:\n";
    int tests[5] = {0, 3, 9, -1, 100};
    for (int idx : tests) {
        try {
            std::cout << "Index " << idx << " -> " << getIdAtIndex(idx) << "\n";
        } catch (const std::exception& e) {
            std::cout << "Index " << idx << " -> error: " << e.what() << "\n";
        }
    }
    return 0;
}