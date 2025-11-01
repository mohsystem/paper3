#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <vector>

// Mock database of subscribed emails. In a real application, this would be a database call.
const std::unordered_set<std::string> emailDatabase = {
    "test@example.com",
    "user1@test.com",
    "fail@example.com",
    "subscriber@newsletter.org"
};

/**
 * @brief Simulates checking if an email exists in the database.
 * @param email The email to check.
 * @return true if the email exists, false otherwise.
 */
bool emailExists(const std::string& email) {
    // SECURITY: In a real application, use a parameterized query or a prepared
    // statement to prevent SQL injection when querying the database.
    return emailDatabase.count(email) > 0;
}

/**
 * @brief Simulates running an external script to unsubscribe an email.
 * @param email The email to unsubscribe.
 * @return 0 on success, a non-zero value on failure.
 */
int runUnsubscribeScript(const std::string& email) {
    // SECURITY: Never build a command string by concatenating user input.
    // This prevents command injection vulnerabilities. Use APIs like execv() 
    // where arguments are passed in an array, not a single parsed string.
    std::cout << "   -> Running unsubscribe script for: " << email << std::endl;

    // Mock logic: script fails if the email contains "fail".
    if (email.find("fail") != std::string::npos) {
        std::cout << "   -> Script failed." << std::endl;
        return -1; // Non-zero indicates failure
    }
    
    std::cout << "   -> Script succeeded." << std::endl;
    return 0; // 0 indicates success
}

/**
 * @brief Checks if an email exists and unsubscribes it.
 * @param email The email address to process.
 * @return 1 if the email existed and the unsubscribe script succeeded.
 *         0 in all other cases (email not found, invalid email, or script failure).
 */
int unsubscribeEmail(const std::string& email) {
    // 1. Input Validation
    const std::regex email_pattern(
        R"(^[a-zA-Z0-9_+&*-]+(?:\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\.)+[a-zA-Z]{2,7}$)"
    );
    if (!std::regex_match(email, email_pattern)) {
        std::cout << "Input '" << email << "' is invalid." << std::endl;
        return 0;
    }

    // 2. Check if email exists in the database.
    if (!emailExists(email)) {
        std::cout << "Email '" << email << "' not found in database." << std::endl;
        return 0;
    }

    std::cout << "Email '" << email << "' found. Attempting to unsubscribe." << std::endl;
    // 3. Call the unsubscribe script.
    int scriptResult = runUnsubscribeScript(email);

    // 4. Check script result and return final status.
    if (scriptResult == 0) {
        return 1; // Script succeeded
    } else {
        return 0; // Script failed
    }
}

int main() {
    std::vector<std::pair<std::string, int>> testCases = {
        {"test@example.com", 1},       // Exists, script will succeed
        {"fail@example.com", 0},       // Exists, script will fail
        {"notfound@example.com", 0},   // Does not exist
        {"invalid-email", 0},          // Invalid format
        {"subscriber@newsletter.org", 1}// Exists, script will succeed
    };

    int i = 1;
    for (const auto& testCase : testCases) {
        std::cout << "--- Test Case " << i++ << ": " << testCase.first << " ---" << std::endl;
        int result = unsubscribeEmail(testCase.first);
        std::cout << "Result: " << result << " (Expected: " << testCase.second << ")" << std::endl;
        std::cout << "Status: " << (result == testCase.second ? "PASSED" : "FAILED") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}