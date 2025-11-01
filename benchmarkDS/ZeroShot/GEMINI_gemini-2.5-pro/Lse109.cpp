#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <regex>

// Regex for a typical, valid username.
const std::regex VALID_USERNAME_REGEX("^[a-zA-Z0-9_-]{1,32}$");
const std::string PASSWD_FILE = "/etc/passwd";

/**
 * Securely retrieves a user's information from /etc/passwd by parsing the file.
 * This avoids command injection vulnerabilities.
 *
 * @param username The username to search for.
 * @return A string containing the user's line from /etc/passwd, or a message if not found or invalid.
 */
std::string getUserInfo(const std::string& username) {
    if (!std::regex_match(username, VALID_USERNAME_REGEX)) {
        return "Error: Invalid username format provided.";
    }

    std::ifstream file(PASSWD_FILE);
    if (!file.is_open()) {
        return "Error: Could not open " + PASSWD_FILE + ". This program is intended for Unix-like systems.";
    }

    std::string line;
    while (std::getline(file, line)) {
        // The format of /etc/passwd is `username:password:UID:GID:GECOS:home:shell`
        size_t first_colon = line.find(':');
        if (first_colon != std::string::npos) {
            if (line.substr(0, first_colon) == username) {
                return line;
            }
        }
    }

    file.close();
    return "User '" + username + "' not found.";
}

int main() {
    std::vector<std::string> testUsers = {
        "root",                 // A user that should exist on most systems
        "nobody",               // Another common user
        "nonexistentuser123",   // A user that should not exist
        "daemon",               // A common system user
        "user;rm-rf/"           // An invalid/malicious username attempt
    };

    for (const auto& user : testUsers) {
        std::cout << "Searching for user: " << user << std::endl;
        std::string result = getUserInfo(user);
        std::cout << "Result: " << result << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}