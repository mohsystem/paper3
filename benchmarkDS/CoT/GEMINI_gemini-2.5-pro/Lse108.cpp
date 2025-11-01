#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <regex>

/**
 * Validates a username to ensure it contains only safe characters.
 * This prevents command injection.
 * Allowed characters are: a-z, A-Z, 0-9, underscore, and hyphen.
 *
 * @param username The username string to validate.
 * @return true if the username is valid, false otherwise.
 */
bool isValidUsername(const std::string& username) {
    if (username.empty()) {
        return false;
    }
    // A regex for typical Linux usernames.
    // It must not contain shell metacharacters.
    const std::regex pattern("^[a-zA-Z0-9_-]{1,32}$");
    return std::regex_match(username, pattern);
}

/**
 * Executes a command and returns its output. This uses popen, which is safer
 * than system() as it avoids shell interpretation of arguments if they are
 * properly escaped/validated, and it allows direct capture of output.
 *
 * @param username The username to look up.
 * @return The output from the 'getent passwd' command.
 */
std::string getUserInfo(const std::string& username) {
    if (!isValidUsername(username)) {
        return "Error: Invalid username format.";
    }

    std::string command = "getent passwd " + username;
    std::array<char, 128> buffer;
    std::string result;
    
    // popen executes a command and opens a pipe to read its output.
    // The "r" means we are reading from the command's stdout.
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

int main() {
    std::vector<std::string> testUsernames = {
        "root",                 // Test Case 1: A valid, existing user
        "nonexistentuser12345", // Test Case 2: A valid but likely non-existing user
        "user name",            // Test Case 3: Invalid format (space)
        "root;whoami",          // Test Case 4: Command injection attempt
        ""                      // Test Case 5: Empty username
    };

    for (const auto& user : testUsernames) {
        std::cout << "---- Testing for username: '" << user << "' ----" << std::endl;
        try {
            std::string result = getUserInfo(user);
            if (result.empty()) {
                std::cout << "No output received (user may not exist or command failed silently)." << std::endl;
            } else {
                std::cout << result;
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        std::cout << "----------------------------------------\n" << std::endl;
    }

    return 0;
}