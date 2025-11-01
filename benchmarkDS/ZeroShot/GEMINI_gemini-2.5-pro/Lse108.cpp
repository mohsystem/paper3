#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <regex>

class Lse108 {
public:
    /**
     * Executes the 'getent passwd <username>' command securely.
     *
     * It validates the username against a strict regex before constructing the command
     * string. This sanitization is the key security measure to prevent command injection
     * when using popen.
     *
     * @param username The username to look up.
     * @return The output of the command, or an error message if the username is invalid.
     */
    static std::string executeGetent(const std::string& username) {
        // A regex pattern for a typical and safe Linux username.
        const std::regex username_pattern("^[a-z_][a-z0-9_-]{0,31}$");
        if (!std::regex_match(username, username_pattern)) {
            return "Error: Invalid username format.\n";
        }

        // The command is safe to build now because the username has been validated.
        std::string command = "getent passwd " + username;
        
        std::array<char, 128> buffer;
        std::string result;
        
        // Use popen to execute the command and open a pipe to read its output.
        // unique_ptr with a custom deleter (pclose) ensures the pipe is closed.
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            return "Error: popen() failed!\n";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        return result;
    }
};

int main() {
    std::string test_usernames[] = {
        "root",                          // Test Case 1: Valid and existing user
        "nobody",                        // Test Case 2: Another valid user
        "thisuserdoesnotexist123",       // Test Case 3: Valid format, but likely non-existent
        "user;whoami",                   // Test Case 4: Command injection attempt (should be rejected)
        "us$er"                          // Test Case 5: Invalid character (should be rejected)
    };

    for (const auto& user : test_usernames) {
        std::cout << "--- Testing with username: '" << user << "' ---" << std::endl;
        std::string result = Lse108::executeGetent(user);
        std::cout << "Output:" << std::endl;
        // An empty result from popen usually means the command produced no output (e.g., user not found).
        if (result.empty() && user.find(';') == std::string::npos && user.find('$') == std::string::npos) {
            std::cout << "User not found or an error occurred." << std::endl;
        } else {
            std::cout << result;
        }
        std::cout << "----------------------------------------\n" << std::endl;
    }

    return 0;
}