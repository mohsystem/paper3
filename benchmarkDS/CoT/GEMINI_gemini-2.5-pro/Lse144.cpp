#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

// Simulates a database of subscribed emails for demonstration purposes.
const std::unordered_set<std::string> emailDatabase = {
    "user.success@example.com",
    "user.fail@example.com",
    "another.success@example.com",
    "test.user@domain.com"
};

/**
 * @brief Simulates an external script that unsubscribes an email.
 * In a real-world scenario, this would be a call to an external service or script.
 * Calling a function is much safer than using system(),
 * which is vulnerable to command injection.
 * 
 * @param email The email to unsubscribe.
 * @return 0 for success, 1 for failure.
 */
int mockUnsubscribeScript(const std::string& email) {
    // This is a mock implementation.
    // It succeeds if the email contains "success" or "test", and fails otherwise.
    if (email.find("success") != std::string::npos || email.find("test") != std::string::npos) {
        std::cout << "-> Mock script: Successfully unsubscribed " << email << std::endl;
        return 0; // Success
    } else {
        std::cout << "-> Mock script: Failed to unsubscribe " << email << std::endl;
        return 1; // Failure
    }
}

/**
 * @brief Checks if an email exists and attempts to unsubscribe it.
 * 
 * @param email The email address to check and unsubscribe.
 * @return 1 if the email exists and the unsubscribe script succeeds, otherwise 0.
 */
int unsubscribeUser(const std::string& email) {
    // Input validation: ensure email is not empty.
    if (email.empty()) {
        std::cout << "Processing '(empty)': Invalid input." << std::endl;
        return 0;
    }

    std::cout << "Processing '" << email << "':" << std::endl;

    // Step 1: Check if the email exists in the database.
    if (emailDatabase.count(email)) {
        std::cout << "-> Email found in the database." << std::endl;
        // Step 2: If it exists, call the unsubscribe script.
        int scriptResult = mockUnsubscribeScript(email);
        
        // Step 3: Check the script's return value.
        if (scriptResult == 0) {
            // Script succeeded, return 1.
            std::cout << "-> Result: 1 (Success)" << std::endl;
            return 1;
        } else {
            // Script failed, return 0.
            std::cout << "-> Result: 0 (Script Failed)" << std::endl;
            return 0;
        }
    } else {
        // Email does not exist, return 0.
        std::cout << "-> Email not found in the database." << std::endl;
        std::cout << "-> Result: 0" << std::endl;
        return 0;
    }
}

int main() {
    std::vector<std::string> testEmails = {
        "user.success@example.com",  // Test case 1: Email exists, script succeeds
        "user.fail@example.com",     // Test case 2: Email exists, script fails
        "not.found@example.com",     // Test case 3: Email does not exist
        "test.user@domain.com",      // Test case 4: Another success case
        ""                           // Test case 5: Invalid empty input
    };

    for (int i = 0; i < testEmails.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        int result = unsubscribeUser(testEmails[i]);
        std::cout << "Final return value: " << result << std::endl;
    }

    return 0;
}