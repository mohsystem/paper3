#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <algorithm>

/**
 * Validates that a string contains only alphanumeric characters.
 * @param s The string to validate.
 * @return true if the string is alphanumeric, false otherwise.
 */
bool isAlphanumeric(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isalnum);
}

/**
 * Processes user input after validation.
 * @param username A string representing the username.
 * @param ageStr A string representing the user's age.
 */
void performOperation(const std::string& username, const std::string& ageStr) {
    // 1. Validate Username
    if (username.empty()) {
        std::cout << "Error: Username cannot be empty." << std::endl;
        return;
    }
    if (username.length() < 3 || username.length() > 20) {
        std::cout << "Error: Username must be between 3 and 20 characters long." << std::endl;
        return;
    }
    if (!isAlphanumeric(username)) {
        std::cout << "Error: Username must contain only alphanumeric characters." << std::endl;
        return;
    }

    // 2. Validate Age
    int age;
    try {
        size_t pos;
        age = std::stoi(ageStr, &pos);
        // Check if the entire string was consumed
        if (pos != ageStr.length()) {
            throw std::invalid_argument("Invalid character in age string");
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: Age must be a valid integer." << std::endl;
        return;
    } catch (const std::out_of_range& e) {
        std::cout << "Error: Age is out of the valid integer range." << std::endl;
        return;
    }

    if (age < 0 || age > 120) {
        std::cout << "Error: Age must be between 0 and 120." << std::endl;
        return;
    }

    // 3. Perform Operation if all validations pass
    std::cout << "Success: Hello " << username << ", you are " << age << " years old." << std::endl;
}

int main() {
    // Main function with 5 test cases
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid input
    std::cout << "Test 1 (Valid):" << std::endl;
    performOperation("Alice", "30");
    std::cout << std::endl;

    // Test Case 2: Invalid age (non-numeric)
    std::cout << "Test 2 (Invalid Age - Text):" << std::endl;
    performOperation("Bob", "twenty");
    std::cout << std::endl;

    // Test Case 3: Invalid age (out of range)
    std::cout << "Test 3 (Invalid Age - Range):" << std::endl;
    performOperation("Charlie", "150");
    std::cout << std::endl;

    // Test Case 4: Invalid username (special characters)
    std::cout << "Test 4 (Invalid Username - Chars):" << std::endl;
    performOperation("David!@", "40");
    std::cout << std::endl;

    // Test Case 5: Invalid username (too short)
    std::cout << "Test 5 (Invalid Username - Length):" << std::endl;
    performOperation("Ed", "25");
    std::cout << std::endl;

    return 0;
}