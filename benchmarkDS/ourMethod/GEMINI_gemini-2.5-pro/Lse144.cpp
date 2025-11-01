#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <vector>

// A mock database of email addresses.
const std::unordered_set<std::string> emailDatabase = {
    "user1@example.com",
    "success@example.com",
    "fail@example.com"
};

const size_t MAX_EMAIL_LENGTH = 254;

/**
 * @brief Validates an email address format and length.
 * @param email The email string to validate.
 * @return true if the email is valid, false otherwise.
 */
bool isValidEmail(const std::string& email) {
    if (email.empty() || email.length() > MAX_EMAIL_LENGTH) {
        return false;
    }
    // This is a basic regex and may not cover all edge cases of RFC 5322.
    const std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$");
    return std::regex_match(email, pattern);
}

/**
 * @brief Checks if an email exists in the mock database.
 * @param email The email to check.
 * @return true if the email exists, false otherwise.
 */
bool emailExistsInDB(const std::string& email) {
    // In a real application, this would query a database.
    return emailDatabase.count(email) > 0;
}

/**
 * @brief Simulates calling an external script to unsubscribe an email.
 * @param email The email to unsubscribe.
 * @return 0 for success, non-zero for failure.
 */
int callUnsubscribeScript(const std::string& email) {
    std::cout << "Simulating call to unsubscribe script for: " << email << std::endl;
    // Security Note: In a real-world application, NEVER use system(). It is vulnerable
    // to command injection. On POSIX systems, use fork() and one of the exec() family
    // functions (e.g., execlp) to pass arguments safely. On Windows, use CreateProcess.
    // Example (POSIX):
    // #include <unistd.h>
    // #include <sys/wait.h>
    // pid_t pid = fork();
    // if (pid == 0) { // Child process
    //     execlp("/path/to/unsubscribe.sh", "unsubscribe.sh", email.c_str(), (char*)NULL);
    //     _exit(127); // exec failed
    // } else if (pid > 0) { // Parent process
    //     int status;
    //     waitpid(pid, &status, 0);
    //     if (WIFEXITED(status)) return WEXITSTATUS(status);
    // }
    // return 1; // Fork failed or other error

    // Mock logic: script fails for a specific email for testing purposes.
    if (email == "fail@example.com") {
        std::cout << "Mock script simulation: FAILED." << std::endl;
        return 1; // Simulate failure
    }

    std::cout << "Mock script simulation: SUCCESS." << std::endl;
    return 0; // Simulate success
}

/**
 * @brief Processes an unsubscribe request based on the specified logic.
 * @param email The email address to process.
 * @return 1 if the email exists and is successfully unsubscribed, 0 otherwise.
 */
int processUnsubscribeRequest(const std::string& email) {
    // 1. Rule#1: Ensure all input is validated.
    if (!isValidEmail(email)) {
        return 0;
    }

    // 2. Check if email exists in the database.
    if (emailExistsInDB(email)) {
        // 3. If it exists, call the unsubscribe script.
        int scriptResult = callUnsubscribeScript(email);
        
        // 4. If the script returns 0, it was a success.
        if (scriptResult == 0) {
            return 1; // Success
        } else {
            return 0; // Script failed
        }
    } else {
        // Email does not exist.
        return 0;
    }
}

int main() {
    std::vector<std::string> testEmails = {
        "success@example.com",  // Case 1: Exists, script succeeds -> should return 1
        "fail@example.com",     // Case 2: Exists, script fails -> should return 0
        "notfound@example.com", // Case 3: Does not exist -> should return 0
        "invalid-email",        // Case 4: Invalid format -> should return 0
        ""                      // Case 5: Empty string -> should return 0
    };

    for (const auto& email : testEmails) {
        std::cout << "\n--- Processing test case for: '" << email << "' ---" << std::endl;
        int result = processUnsubscribeRequest(email);
        std::cout << "Result for '" << email << "': " << result << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    return 0;
}