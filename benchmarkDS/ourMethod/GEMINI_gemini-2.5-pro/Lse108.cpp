#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <regex>
#include <vector>
#include <sys/wait.h> // For WIFEXITED, WEXITSTATUS

/**
 * Validates a username. A valid username must be 1-32 chars and contain
 * only alphanumeric, dot, underscore, or hyphen characters.
 * @param username The username to validate.
 * @return True if the username is valid, false otherwise.
 */
bool isValidUsername(const std::string& username) {
    if (username.empty() || username.length() > 32) {
        return false;
    }
    // Rule #1: Validate input against a strict pattern.
    static const std::regex pattern("^[a-zA-Z0-9._-]+$");
    return std::regex_match(username, pattern);
}

/**
 * Executes 'getent passwd <username>' and returns its output.
 * The prompt requires using a function like system(), so this implementation
 * uses popen(), which invokes a shell but allows for output capture.
 * The username is strictly validated to prevent command injection.
 *
 * @param username The username to look up.
 * @return The output of the command, or an error message.
 */
std::string getUserInfo(const std::string& username) {
    if (!isValidUsername(username)) {
        return "Error: Username is invalid.";
    }

    std::string command = "getent passwd " + username;
    std::string result;
    std::array<char, 256> buffer;

    // Rule #2: Use potentially dangerous functions with caution.
    // popen() invokes /bin/sh. Security relies on the strict validation above.
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "Error: popen() failed!";
    }

    // Rule #3: Ensure buffer operations adhere to boundaries.
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    int status = pclose(pipe);
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0) {
            // Command was successful
            return result;
        } else {
            // Command failed, result might be empty.
            if (result.empty()) {
                return "User not found or an error occurred.";
            }
            return result; // Return partial result if any
        }
    }

    return "Error: Command terminated abnormally.";
}

int main() {
    std::vector<std::string> testUsernames = {
        "root",                 // Test case 1: Valid, existing user
        "nonexistentuser12345", // Test case 2: Valid, non-existing user
        "user; whoami",         // Test case 3: Invalid user with command injection attempt
        "",                     // Test case 4: Empty string
        "daemon"                // Test case 5: Another valid, existing user
    };

    for (const auto& user : testUsernames) {
        std::cout << "--- Testing for username: '" << user << "' ---" << std::endl;
        std::string result = getUserInfo(user);
        std::cout << "Result:\n" << result << std::endl;
        std::cout << "----------------------------------------\n" << std::endl;
    }

    return 0;
}