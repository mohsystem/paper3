/*
Chain-of-Through process:
1) Problem understanding: Implement getValueFromArray and main that reads an int, invokes it with id_sequence, exits with returned value.
2) Security requirements: Validate inputs, bounds check, sanitize exit codes, handle malformed input.
3) Secure coding: Use exceptions for invalid cases; catch and handle in main; avoid UB.
4) Code review: Verified checks and sanitization.
5) Secure output: Test mode when no input is provided.
*/
#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

int getValueFromArray(const std::vector<int>& arr, std::size_t size, int index) {
    if (size > arr.size()) throw std::invalid_argument("Invalid size");
    if (index < 0 || static_cast<std::size_t>(index) >= size)
        throw std::out_of_range("Index out of range");
    return arr[static_cast<std::size_t>(index)];
}

static int toExitCode(int value) {
    return static_cast<unsigned int>(value) & 0xFF; // 0..255
}

static void runTests() {
    std::vector<int> id_sequence {10, 20, 30, 40, 50, 60, 70};
    std::size_t size = id_sequence.size();
    int tests[5] = {0, 3, 4, 6, 10}; // last is out-of-bounds
    std::cout << "Test mode: demonstrating 5 test cases\n";
    for (int idx : tests) {
        try {
            int v = getValueFromArray(id_sequence, size, idx);
            std::cout << "Index " << idx << " -> " << v << "\n";
        } catch (const std::exception& e) {
            std::cout << "Index " << idx << " -> error: " << e.what() << "\n";
        }
    }
}

int main() {
    std::vector<int> id_sequence {10, 20, 30, 40, 50, 60, 70};
    std::size_t size = id_sequence.size();

    // Attempt to read an integer index
    int idx;
    if (std::cin >> idx) {
        try {
            int value = getValueFromArray(id_sequence, size, idx);
            return toExitCode(value);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    } else {
        // No valid integer provided; run 5 test cases
        runTests();
        return 0;
    }
}