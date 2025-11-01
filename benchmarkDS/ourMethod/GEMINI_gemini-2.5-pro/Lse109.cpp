#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <regex>

// A whitelist pattern for valid usernames.
// Allows alphanumeric characters, underscore, dot, and hyphen.
// This helps prevent command injection and other injection attacks.
const std::regex USERNAME_PATTERN("^[a-zA-Z0-9_.-]+$");
const std::string PASSWD_FILE = "/etc/passwd";

/**
 * Finds and returns the line for a given username from /etc/passwd.
 * This implementation reads the file directly to avoid command injection vulnerabilities
 * that would arise from executing external shell commands with user input.
 *
 * @param username The username to search for.
 * @return The line from /etc/passwd, or an empty string if not found or invalid.
 */
std::string getUserInfo(const std::string& username) {
    if (!std::regex_match(username, USERNAME_PATTERN)) {
        std::cerr << "Error: Invalid username format provided." << std::endl;
        return "";
    }

    std::ifstream passwd_file(PASSWD_FILE);
    if (!passwd_file.is_open()) {
        std::cerr << "Error: Could not open " << PASSWD_FILE << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(passwd_file, line)) {
        // /etc/passwd format is username:password:UID:GID:...
        size_t first_colon = line.find(':');
        if (first_colon != std::string::npos) {
            if (line.substr(0, first_colon) == username) {
                passwd_file.close();
                return line;
            }
        }
    }

    passwd_file.close();
    return "";
}

void runTests() {
    std::vector<std::string> testUsernames = {
        "root",                 // Test case 1: A common existing user
        "daemon",               // Test case 2: Another common user
        "nonexistentuser12345", // Test case 3: A non-existent user
        "root;whoami",          // Test case 4: A malicious input pattern
        ""                      // Test case 5: An empty input
    };

    for (const auto& testUser : testUsernames) {
        std::cout << "Searching for user: \"" << testUser << "\"" << std::endl;
        std::string result = getUserInfo(testUser);
        if (!result.empty()) {
            std::cout << "Found: " << result << std::endl;
        } else {
            std::cout << "Result: Not found or invalid input." << std::endl;
        }
        std::cout << "---------------------------------" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <username>" << std::endl;
        std::cout << "\n--- Running Internal Test Cases ---" << std::endl;
        runTests();
        return 1;
    }

    std::string username(argv[1]);
    std::string userInfo = getUserInfo(username);

    if (!userInfo.empty()) {
        std::cout << userInfo << std::endl;
    } else {
        std::cout << "User '" << username << "' not found or input was invalid." << std::endl;
    }

    return 0;
}