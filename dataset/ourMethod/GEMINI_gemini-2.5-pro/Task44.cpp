#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <stdexcept>

// Validates a username.
// Rules: 3 to 16 characters long, alphanumeric characters only.
bool isValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 16) {
        return false;
    }
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

// Validates an age string.
// Rules: Must be a valid integer between 0 and 120 (inclusive).
bool isValidAge(const std::string& ageStr, int& age) {
    try {
        size_t pos = 0;
        long long age_long = std::stoll(ageStr, &pos);

        // Ensure the entire string was a number.
        if (pos != ageStr.length()) {
            return false;
        }

        // Check the logical range for an age.
        if (age_long < 0 || age_long > 120) {
            return false;
        }

        age = static_cast<int>(age_long);
        return true;
    } catch (const std::invalid_argument&) {
        // Not a number
        return false;
    } catch (const std::out_of_range&) {
        // Number is too large or too small for long long
        return false;
    }
}

// A dummy function to simulate processing valid data.
void processUserData(const std::string& username, int age) {
    std::cout << "Processing data for user '" << username << "' with age " << age << "." << std::endl;
    // In a real application, business logic would go here.
    std::cout << "Operation successful." << std::endl;
}

// Helper function to run a single test case.
void runTestCase(const std::string& username_input, const std::string& age_input) {
    std::cout << "--- Test Case ---" << std::endl;
    std::cout << "Input Username: \"" << username_input << "\", Input Age: \"" << age_input << "\"" << std::endl;

    int age = 0;
    bool is_username_valid = isValidUsername(username_input);
    bool is_age_valid = isValidAge(age_input, age);

    if (is_username_valid && is_age_valid) {
        std::cout << "Validation successful." << std::endl;
        processUserData(username_input, age);
    } else {
        std::cout << "Validation failed." << std::endl;
        if (!is_username_valid) {
            std::cout << "  - Invalid username: Must be 3-16 alphanumeric characters." << std::endl;
        }
        if (!is_age_valid) {
            std::cout << "  - Invalid age: Must be a whole number between 0 and 120." << std::endl;
        }
    }
    std::cout << std::endl;
}

int main() {
    // A series of test cases to demonstrate input validation.
    runTestCase("testuser1", "25");       // 1. Valid case
    runTestCase("sh", "30");              // 2. Invalid username (too short)
    runTestCase("longusername1234567", "40"); // 3. Invalid username (too long)
    runTestCase("user_name", "35");       // 4. Invalid username (non-alphanumeric character)
    runTestCase("testuser2", "150");      // 5. Invalid age (out of range)
    runTestCase("testuser3", "-5");       // 6. Invalid age (out of range)
    runTestCase("testuser4", "twenty");   // 7. Invalid age (not a number)
    runTestCase("testuser5", "25a");      // 8. Invalid age (contains non-digit characters)
    return 0;
}