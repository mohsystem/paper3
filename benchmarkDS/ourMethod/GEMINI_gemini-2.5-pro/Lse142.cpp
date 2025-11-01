#include <iostream>
#include <string>
#include <set>
#include <regex>

// Using a std::set for efficient lookups (O(log n) time complexity).
// It also handles memory management for the strings it contains.
class UnsubscribeService {
private:
    std::set<std::string> emailDatabase;

    /**
     * @brief Validates an email address against a standard regex and length constraints.
     * @param email The email address to validate.
     * @return true if the email is valid, false otherwise.
     */
    bool isValidEmail(const std::string& email) {
        if (email.empty()) {
            return false;
        }
        // Limit email length to a reasonable size to prevent abuse
        if (email.length() > 254) {
            return false;
        }
        // A common regex for email validation
        const std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$");
        return std::regex_match(email, pattern);
    }

public:
    UnsubscribeService() {
        // Pre-populate the database for demonstration
        emailDatabase.insert("test@example.com");
        emailDatabase.insert("user1@domain.com");
        emailDatabase.insert("another.user@service.net");
    }

    /**
     * @brief Unsubscribes an email address by removing it from the database.
     * @param email The email address to unsubscribe.
     * @return A status message indicating the result of the operation.
     */
    std::string unsubscribe(const std::string& email) {
        // Rule #1: Ensure all input is validated.
        if (!isValidEmail(email)) {
            return "Invalid email address format.";
        }

        // The core logic of the function
        if (emailDatabase.count(email) > 0) {
            emailDatabase.erase(email);
            return "Successfully unsubscribed " + email + ".";
        } else {
            return "Email address not found in the database.";
        }
    }
};

int main() {
    UnsubscribeService service;
    
    std::cout << "--- Test Cases ---" << std::endl;

    // Test Case 1: A valid email that exists in the database.
    std::string email1 = "test@example.com";
    std::cout << "Attempting to unsubscribe: " << email1 << std::endl;
    std::cout << "Result: " << service.unsubscribe(email1) << std::endl;
    std::cout << "Re-checking: " << service.unsubscribe(email1) << std::endl; // Should now be not found
    std::cout << std::endl;

    // Test Case 2: A valid email that does not exist in the database.
    std::string email2 = "nonexistent@example.com";
    std::cout << "Attempting to unsubscribe: " << email2 << std::endl;
    std::cout << "Result: " << service.unsubscribe(email2) << std::endl;
    std::cout << std::endl;

    // Test Case 3: An invalid email address.
    std::string email3 = "invalid-email";
    std::cout << "Attempting to unsubscribe: " << email3 << std::endl;
    std::cout << "Result: " << service.unsubscribe(email3) << std::endl;
    std::cout << std::endl;

    // Test Case 4: An empty string input.
    std::string email4 = "";
    std::cout << "Attempting to unsubscribe: \"\"" << std::endl;
    std::cout << "Result: " << service.unsubscribe(email4) << std::endl;
    std::cout << std::endl;

    // Test Case 5: A very long but valid-looking email.
    std::string email5(300, 'a');
    email5 += "@example.com";
    std::cout << "Attempting to unsubscribe: [a very long email]" << std::endl;
    std::cout << "Result: " << service.unsubscribe(email5) << std::endl;
    std::cout << std::endl;

    return 0;
}