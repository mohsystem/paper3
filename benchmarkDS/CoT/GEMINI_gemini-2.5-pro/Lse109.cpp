#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm> // For std::all_of

/**
 * Finds and returns a user's information from /etc/passwd.
 * This function safely reads the file line by line to prevent command injection
 * vulnerabilities that would arise from executing shell commands with user input.
 *
 * @param username The username to search for.
 * @return The full line from /etc/passwd for the user, or a "not found" message.
 */
std::string getUserInfo(const std::string& username) {
    // Basic validation to prevent invalid input
    if (username.empty() || username.find('/') != std::string::npos || username.find(';') != std::string::npos) {
        return "Invalid username provided.";
    }

    std::ifstream passwdFile("/etc/passwd");
    if (!passwdFile.is_open()) {
        return "Error: Could not open /etc/passwd";
    }

    std::string line;
    std::string searchPrefix = username + ":";

    while (std::getline(passwdFile, line)) {
        // `rfind` with a starting position of 0 is a way to check for a prefix.
        if (line.rfind(searchPrefix, 0) == 0) {
            passwdFile.close();
            return line;
        }
    }

    passwdFile.close();
    return "User '" + username + "' not found.";
}

int main(int argc, char* argv[]) {
    // The original prompt asked for command-line argument processing.
    // If arguments are provided, we use them. Otherwise, we run test cases.
    if (argc > 1) {
        std::string username = argv[1];
        std::cout << "Searching for user from command line: " << username << std::endl;
        std::cout << getUserInfo(username) << std::endl;
    } else {
        // Running 5 test cases as requested.
        std::cout << "--- Running Test Cases ---" << std::endl;
        std::vector<std::string> testUsers = {"root", "daemon", "nobody", "nonexistentuser123", "adm"};

        for (const auto& user : testUsers) {
            std::cout << "\nSearching for user: " << user << std::endl;
            std::string result = getUserInfo(user);
            std::cout << "Result: " << result << std::endl;
        }

        std::cout << "\n--- Test Cases Finished ---" << std::endl;
        std::cout << "You can also run this program with a command-line argument, e.g., ./your_program_name root" << std::endl;
    }

    return 0;
}