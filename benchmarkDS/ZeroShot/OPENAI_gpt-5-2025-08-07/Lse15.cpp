#include <iostream>
#include <string>
#include <limits>
#include <stdexcept>

static const int ID_SEQUENCE[] = {101, 203, 307, 409, 512, 618, 723, 834, 945, 1056};
static const std::size_t ID_SEQUENCE_LEN = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

// Function: takes index and returns the value at that index
int getValueAtIndex(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= ID_SEQUENCE_LEN) {
        throw std::out_of_range("Index out of bounds.");
    }
    return ID_SEQUENCE[index];
}

static void prompt_once() {
    std::cout << "Enter an index (0-" << (ID_SEQUENCE_LEN - 1) << "): ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "No input provided.\n";
        return;
    }
    // Trim whitespace
    auto start = line.find_first_not_of(" \t\r\n");
    auto end = line.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) {
        std::cout << "Empty input.\n";
        return;
    }
    std::string trimmed = line.substr(start, end - start + 1);
    try {
        std::size_t pos = 0;
        long long val = std::stoll(trimmed, &pos, 10);
        if (pos != trimmed.size()) {
            std::cout << "Invalid number format.\n";
            return;
        }
        if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) {
            std::cout << "Input out of integer range.\n";
            return;
        }
        int idx = static_cast<int>(val);
        try {
            int result = getValueAtIndex(idx);
            std::cout << "Value at index " << idx << " is: " << result << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    } catch (const std::exception&) {
        std::cout << "Invalid number format.\n";
    }
}

int main() {
    // Prompt the user for an index
    prompt_once();

    // Five test cases
    int tests[5] = {0, 3, 9, -1, 100};
    std::cout << "Running 5 test cases:\n";
    for (int idx : tests) {
        try {
            std::cout << "Index " << idx << " -> " << getValueAtIndex(idx) << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Index " << idx << " -> Error: " << ex.what() << "\n";
        }
    }
    return 0;
}