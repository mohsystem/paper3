#include <iostream>
#include <string>
#include <stdexcept>
#include <regex>
#include <vector>

/**
 * @brief Validates user input and returns a greeting message.
 * @param name The user's name. Must be 2-50 chars, letters and spaces only.
 * @param age The user's age. Must be between 0 and 120.
 * @return A greeting string if the input is valid.
 * @throw std::invalid_argument if the input is invalid.
 */
std::string processUserInput(const std::string& name, int age) {
    // Validate name
    if (name.empty() || std::all_of(name.begin(), name.end(), isspace)) {
        throw std::invalid_argument("Name cannot be null or empty.");
    }
    if (name.length() < 2 || name.length() > 50) {
        throw std::invalid_argument("Name must be between 2 and 50 characters.");
    }
    // Regex to allow only letters and spaces
    if (!std::regex_match(name, std::regex("^[a-zA-Z\\s]+$"))) {
        throw std::invalid_argument("Name can only contain letters and spaces.");
    }

    // Validate age
    if (age < 0 || age > 120) {
        throw std::invalid_argument("Age must be between 0 and 120.");
    }
    
    return "Hello, " + name + "! You are " + std::to_string(age) + " years old.";
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid input
    try {
        std::cout << "Test Case 1 (Valid): " << processUserInput("Alice Smith", 30) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test Case 1 (Valid): FAILED - " << e.what() << std::endl;
    }

    // Test Case 2: Invalid name (with numbers)
    try {
        std::cout << "Test Case 2 (Invalid Name): " << processUserInput("Bob123", 40) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test Case 2 (Invalid Name): PASSED - " << e.what() << std::endl;
    }

    // Test Case 3: Invalid age (too high)
    try {
        std::cout << "Test Case 3 (Invalid Age): " << processUserInput("Charlie Brown", 150) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test Case 3 (Invalid Age): PASSED - " << e.what() << std::endl;
    }

    // Test Case 4: Invalid name (empty)
    try {
        std::cout << "Test Case 4 (Empty Name): " << processUserInput("  ", 25) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test Case 4 (Empty Name): PASSED - " << e.what() << std::endl;
    }

    // Test Case 5: Invalid name (special characters)
    try {
        std::cout << "Test Case 5 (Special Chars): " << processUserInput("Eve <script>", 22) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test Case 5 (Special Chars): PASSED - " << e.what() << std::endl;
    }

    std::cout << "--- Test Cases Finished ---" << std::endl;
    return 0;
}