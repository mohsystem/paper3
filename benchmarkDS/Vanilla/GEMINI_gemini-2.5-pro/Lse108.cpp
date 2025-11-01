#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <cstdio> // For remove()
#include <vector>

/**
 * @brief Executes 'getent passwd <username>' using system() call and captures the output.
 *
 * The command's output is redirected to a temporary file, which is then read into a string.
 * The temporary file is deleted afterwards.
 *
 * @param username The username to look up.
 * @return A string containing the command's output. Returns an empty string if the user
 *         is not found or an error occurs.
 */
std::string getUserInfo(const std::string& username) {
    // Basic sanitization: check for shell-sensitive characters
    if (username.find_first_of("`$\\;&|<>") != std::string::npos) {
        return "";
    }

    const std::string temp_filename = "cpp_getent_output.tmp";
    // Quote username to handle potential spaces or special characters safely
    std::string command = "getent passwd \"" + username + "\" > " + temp_filename;

    // Execute the command via the system() call
    system(command.c_str());

    std::ifstream file(temp_filename);
    std::string content;
    
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        file.close();
    }
    
    // Clean up the temporary file
    std::remove(temp_filename.c_str());

    return content;
}

int main() {
    std::vector<std::string> test_usernames;
    test_usernames.push_back("root");
    test_usernames.push_back("daemon");
    test_usernames.push_back("nonexistentuser12345");

    const char* currentUser = std::getenv("USER");
    if (currentUser != nullptr) {
        test_usernames.push_back(currentUser);
    } else {
        test_usernames.push_back("nobody"); // Fallback
    }
    
    test_usernames.push_back("");

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& user : test_usernames) {
        std::cout << "Testing for user: \"" << user << "\"" << std::endl;
        std::string output = getUserInfo(user);
        if (output.empty()) {
            std::cout << "Output: <No output or user not found>" << std::endl;
        } else {
            std::cout << "Output: " << output;
        }
        std::cout << "-------------------------" << std::endl;
    }

    return 0;
}