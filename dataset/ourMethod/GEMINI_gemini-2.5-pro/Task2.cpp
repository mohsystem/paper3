#include <iostream>
#include <string>
#include <vector>
#include <cstdio> // For snprintf

/**
 * @brief Creates a phone number string from a vector of 10 integers.
 *
 * This function validates that the input vector contains exactly 10 integers,
 * each between 0 and 9. If the input is valid, it formats them into a
 * phone number string like "(123) 456-7890".
 *
 * @param numbers A constant reference to a vector of integers.
 * @return A std::string containing the formatted phone number. Returns an empty
 *         string if the input is invalid.
 */
std::string createPhoneNumber(const std::vector<int>& numbers) {
    // Rule #3: Validate input length
    if (numbers.size() != 10) {
        return ""; // Invalid size, return empty string as an error indicator
    }

    // Rule #3: Validate input range for each element
    for (int num : numbers) {
        if (num < 0 || num > 9) {
            return ""; // Invalid digit, return empty string
        }
    }

    // Rule #4: Use a buffer with a known, safe size.
    // The format "(%d%d%d) %d%d%d-%d%d%d%d" plus null terminator requires 15 characters.
    char buffer[15];

    // Rule #1 & #4: Use snprintf for safe, bounded string formatting.
    int written = snprintf(buffer, sizeof(buffer), "(%d%d%d) %d%d%d-%d%d%d%d",
                           numbers[0], numbers[1], numbers[2],
                           numbers[3], numbers[4], numbers[5],
                           numbers[6], numbers[7], numbers[8], numbers[9]);

    // Check snprintf's return value for errors or truncation.
    if (written < 0 || static_cast<size_t>(written) >= sizeof(buffer)) {
        // This case is highly unlikely with validated single-digit inputs
        // but is good practice for robust error handling.
        return "";
    }

    return std::string(buffer);
}

void run_test_case(const std::string& test_name, const std::vector<int>& input) {
    std::cout << "Test Case: " << test_name << std::endl;
    std::cout << "Input: {";
    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << input[i] << (i == input.size() - 1 ? "" : ", ");
    }
    std::cout << "}" << std::endl;
    std::string result = createPhoneNumber(input);
    if (result.empty()) {
        std::cout << "Output: Invalid input" << std::endl;
    } else {
        std::cout << "Output: \"" << result << "\"" << std::endl;
    }
    std::cout << "------------------------" << std::endl;
}

int main() {
    // Test Case 1: Standard valid input
    run_test_case("Standard", {1, 2, 3, 4, 5, 6, 7, 8, 9, 0});

    // Test Case 2: All zeros
    run_test_case("All Zeros", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

    // Test Case 3: All nines
    run_test_case("All Nines", {9, 9, 9, 9, 9, 9, 9, 9, 9, 9});

    // Test Case 4: Invalid size (too short)
    run_test_case("Invalid Size", {1, 2, 3});

    // Test Case 5: Invalid digit (out of range)
    run_test_case("Invalid Digit", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    return 0;
}