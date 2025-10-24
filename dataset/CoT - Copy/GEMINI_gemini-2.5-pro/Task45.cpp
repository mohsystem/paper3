#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

/**
 * @brief Performs division on two numbers provided as strings.
 *
 * It handles potential errors like invalid number formats (std::invalid_argument),
 * values out of range (std::out_of_range), and division by zero (std::runtime_error).
 *
 * @param numeratorStr The string representation of the numerator.
 * @param denominatorStr The string representation of the denominator.
 * @return A string containing the result or an error message.
 */
std::string performDivision(const std::string& numeratorStr, const std::string& denominatorStr) {
    try {
        double numerator = std::stod(numeratorStr);
        double denominator = std::stod(denominatorStr);

        if (denominator == 0.0) {
            throw std::runtime_error("Cannot divide by zero.");
        }

        double result = numerator / denominator;
        return "Result: " + std::to_string(result);

    } catch (const std::invalid_argument& e) {
        return "Error: Invalid number format. Please provide valid numbers.";
    } catch (const std::out_of_range& e) {
        return "Error: Number is out of range for a double.";
    } catch (const std::runtime_error& e) {
        return std::string("Error: ") + e.what();
    } catch (...) {
        return "An unexpected error occurred.";
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Valid division
    std::cout << "Test 1 ('10', '2'): " << performDivision("10", "2") << std::endl;

    // Test Case 2: Division by zero
    std::cout << "Test 2 ('5', '0'): " << performDivision("5", "0") << std::endl;

    // Test Case 3: Invalid numerator
    std::cout << "Test 3 ('abc', '5'): " << performDivision("abc", "5") << std::endl;

    // Test Case 4: Invalid denominator
    std::cout << "Test 4 ('10', 'xyz'): " << performDivision("10", "xyz") << std::endl;

    // Test Case 5: Valid floating-point division
    std::cout << "Test 5 ('7.5', '1.5'): " << performDivision("7.5", "1.5") << std::endl;

    return 0;
}