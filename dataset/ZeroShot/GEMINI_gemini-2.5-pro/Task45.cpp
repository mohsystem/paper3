#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <utility>
#include <limits>
#include <cmath> // For std::isnan

/**
 * Securely converts two string inputs to doubles and performs division.
 * It handles empty inputs, non-numeric formats, and division by zero.
 *
 * @param numStr1 The string representing the numerator.
 * @param numStr2 The string representing the denominator.
 * @return The result of the division, or NaN if an error occurs.
 */
double performDivision(const std::string& numStr1, const std::string& numStr2) {
    try {
        // 1. Validate for empty inputs
        if (numStr1.empty() || numStr2.empty()) {
            throw std::invalid_argument("Input strings cannot be empty.");
        }

        // 2. Convert strings to numbers, handling format errors
        // std::stod can throw std::invalid_argument or std::out_of_range
        size_t pos1, pos2;
        double numerator = std::stod(numStr1, &pos1);
        double denominator = std::stod(numStr2, &pos2);

        // Ensure the entire string was consumed, preventing inputs like "123xyz"
        if (pos1 != numStr1.length() || pos2 != numStr2.length()) {
             throw std::invalid_argument("Invalid characters found in number string.");
        }

        // 3. Check for division by zero
        if (denominator == 0.0) {
            throw std::runtime_error("Cannot divide by zero.");
        }

        return numerator / denominator;

    } catch (const std::invalid_argument& e) {
        // Catches errors from std::stod for invalid formats or our custom error
        std::cerr << "Error: Invalid number format provided. " << e.what() << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
    } catch (const std::out_of_range& e) {
        // Catches errors from std::stod when value is too large/small for a double
        std::cerr << "Error: Number is out of range. " << e.what() << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
    } catch (const std::runtime_error& e) {
        // Catches our custom division-by-zero error
        std::cerr << "Error: " << e.what() << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
    } catch (...) {
        // A general catch-all for any other unexpected errors
        std::cerr << "An unexpected error occurred." << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
    }
}

int main() {
    // --- Test Cases ---
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"100", "20"},      // Test Case 1: Valid input
        {"10", "0"},        // Test Case 2: Division by zero
        {"abc", "5"},       // Test Case 3: Invalid number format
        {"50", ""},         // Test Case 4: Empty string input
        {"12.5xyz", "2.5"}  // Test Case 5: Valid start, but with trailing characters
    };

    int testNum = 1;
    for (const auto& tc : testCases) {
        std::cout << "Test Case " << testNum++ << ": Valid input ('" << tc.first << "', '" << tc.second << "')" << std::endl;
        double result = performDivision(tc.first, tc.second);
        if (!std::isnan(result)) {
            std::cout << "Result: " << result << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}