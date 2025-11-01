#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

/**
 * @brief Simulates a script that unsubscribes an email.
 * @param email The email to unsubscribe.
 * @return 0 for success, non-zero for failure.
 */
int unsubscribeScript(const std::string& email) {
    std::cout << "-> Calling unsubscribe script for: " << email << std::endl;
    // Simulate success for emails containing "success", failure otherwise.
    if (email.find("success") != std::string::npos) {
        std::cout << "-> Script returned 0 (success)." << std::endl;
        return 0;
    } else {
        std::cout << "-> Script returned 1 (failure)." << std::endl;
        return 1;
    }
}

/**
 * @brief Checks if an email exists and attempts to unsubscribe if it does.
 * @param email The email to check.
 * @param database A set representing the database of emails.
 * @return 1 if email exists and unsubscribe is successful, 0 otherwise.
 */
int checkAndUnsubscribe(const std::string& email, const std::unordered_set<std::string>& database) {
    std::cout << "Checking for email: " << email << std::endl;
    if (database.find(email) == database.end()) {
        std::cout << "Email not found in database." << std::endl;
        return 0; // Email doesn't exist
    }

    std::cout << "Email found in database." << std::endl;
    if (unsubscribeScript(email) == 0) {
        return 1; // Unsubscribe script was successful
    } else {
        return 0; // Unsubscribe script failed
    }
}

int main() {
    // Mock database
    std::unordered_set<std::string> emailDatabase = {
        "user.success@example.com",
        "user.fail@example.com",
        "another.success@example.com"
    };

    // Test cases
    std::vector<std::string> testEmails = {
        "not.in.db@example.com",        // Test Case 1: Email not in DB
        "user.success@example.com",     // Test Case 2: Email in DB, script succeeds
        "user.fail@example.com",        // Test Case 3: Email in DB, script fails
        "another.success@example.com",  // Test Case 4: Another email in DB, script succeeds
        "another.not.in.db@email.com"   // Test Case 5: Another email not in DB
    };

    for (int i = 0; i < testEmails.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        int result = checkAndUnsubscribe(testEmails[i], emailDatabase);
        std::cout << "Final Result: " << result << std::endl << std::endl;
    }

    return 0;
}