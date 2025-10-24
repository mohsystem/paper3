#include <iostream>
#include <string>
#include <stdexcept>
#include <optional>

/**
 * Attempts to parse two strings as doubles and divide the first by the second.
 * Handles potential std::invalid_argument from stod and manually throws
 * a std::runtime_error for division by zero.
 *
 * @param a The numerator as a string.
 * @param b The denominator as a string.
 * @return An std::optional<double> containing the result if successful,
 *         or an empty optional if an error occurred.
 */
std::optional<double> divideStrings(const std::string& a, const std::string& b) {
    try {
        size_t pos_a, pos_b;
        double numerator = std::stod(a, &pos_a);
        double denominator = std::stod(b, &pos_b);

        // Check if the entire string was consumed by stod
        if (pos_a != a.length() || pos_b != b.length()) {
             throw std::invalid_argument("String contains non-numeric characters.");
        }

        if (denominator == 0.0) {
            throw std::runtime_error("Division by zero is not allowed.");
        }

        return numerator / denominator;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid number format. " << e.what() << std::endl;
        return std::nullopt;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Number is out of range for a double. " << e.what() << std::endl;
        return std::nullopt;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Valid division
    std::cout << "\nTest Case 1: divideStrings(\"10\", \"2\")" << std::endl;
    auto result1 = divideStrings("10", "2");
    if (result1.has_value()) {
        std::cout << "Result: " << result1.value() << std::endl;
    }

    // Test Case 2: Division by zero
    std::cout << "\nTest Case 2: divideStrings(\"5\", \"0\")" << std::endl;
    auto result2 = divideStrings("5", "0");
    if (result2.has_value()) {
        std::cout << "Result: " << result2.value() << std::endl;
    }

    // Test Case 3: Invalid numerator
    std::cout << "\nTest Case 3: divideStrings(\"abc\", \"5\")" << std::endl;
    auto result3 = divideStrings("abc", "5");
    if (result3.has_value()) {
        std::cout << "Result: " << result3.value() << std::endl;
    }

    // Test Case 4: Invalid denominator
    std::cout << "\nTest Case 4: divideStrings(\"10\", \"xyz\")" << std::endl;
    auto result4 = divideStrings("10", "xyz");
    if (result4.has_value()) {
        std::cout << "Result: " << result4.value() << std::endl;
    }

    // Test Case 5: Valid floating-point division
    std::cout << "\nTest Case 5: divideStrings(\"7.5\", \"2.5\")" << std::endl;
    auto result5 = divideStrings("7.5", "2.5");
    if (result5.has_value()) {
        std::cout << "Result: " << result5.value() << std::endl;
    }

    return 0;
}