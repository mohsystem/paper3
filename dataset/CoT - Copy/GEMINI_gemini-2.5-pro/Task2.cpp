#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Accepts a vector of 10 integers (between 0 and 9) and returns a string
 * of those numbers in the form of a phone number.
 *
 * @param numbers A constant reference to a vector of 10 integers.
 * @return A formatted phone number string, e.g., "(123) 456-7890".
 */
std::string createPhoneNumber(const std::vector<int>& numbers) {
    // Input validation to ensure the vector has the correct size.
    if (numbers.size() != 10) {
        throw std::invalid_argument("Input vector must contain exactly 10 integers.");
    }
    
    // Using snprintf for safe, formatted string creation into a fixed-size buffer.
    // The buffer size is 15: 14 characters for "(XXX) XXX-XXXX" and 1 for the null terminator.
    char buf[15];
    int written = snprintf(buf, sizeof(buf), "(%d%d%d) %d%d%d-%d%d%d%d",
             numbers[0], numbers[1], numbers[2],
             numbers[3], numbers[4], numbers[5],
             numbers[6], numbers[7], numbers[8], numbers[9]);

    // Check for snprintf errors (though unlikely here with valid input)
    if (written < 0 || written >= sizeof(buf)) {
        return "Error creating phone number string.";
    }

    return std::string(buf);
}

int main() {
    // 5 Test Cases
    std::vector<std::vector<int>> testCases = {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 9, 8, 7, 6, 5, 4, 3, 2, 1},
        {5, 5, 5, 8, 6, 7, 5, 3, 0, 9},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        try {
            std::string phoneNumber = createPhoneNumber(testCases[i]);
            std::cout << "Test Case " << i + 1 << ": " << phoneNumber << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Test Case " << i + 1 << ": Error - " << e.what() << std::endl;
        }
    }

    return 0;
}