#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // For system()
#include <cctype>  // For isalnum()

/**
 * Validates a path string to ensure it only contains a whitelist of safe characters.
 * This is a crucial security step to prevent command injection when using system().
 *
 * @param path The path string to validate.
 * @return True if the path is considered secure, false otherwise.
 */
bool is_path_secure(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    // A whitelist of allowed characters for a path. This prevents shell
    // metacharacters like ;, &, |, $, ``, (), <, >, etc.
    for (char c : path) {
        if (!isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_') {
            return false;
        }
    }
    return true;
}

/**
 * Executes the 'ls' command using system().
 * It includes a mandatory security check on the input directory to prevent
 * command injection vulnerabilities.
 *
 * @param directory The directory path to list.
 * @return The exit code from system(), or -1 if input validation fails.
 */
int executeLsCommand(const std::string& directory) {
    std::cout << "--- Executing 'ls " << directory << "' ---" << std::endl;

    // Security Check: Validate the input before constructing the command string.
    if (!is_path_secure(directory)) {
        std::cerr << "Error: Invalid or potentially insecure path provided: " << directory << std::endl;
        std::cout << "------------------------------------" << std::endl;
        return -1;
    }

    // Construct the command string after input has been validated.
    std::string command = "ls " + directory;

    // Execute the command using system().
    int return_code = std::system(command.c_str());

    std::cout << "Exited with code: " << return_code << std::endl;
    std::cout << "------------------------------------" << std::endl;
    
    return return_code;
}

int main() {
    // Test Case 1: Current directory
    executeLsCommand(".");

    // Test Case 2: A common, valid directory
    executeLsCommand("/tmp");

    // Test Case 3: A validly-formatted but non-existent directory
    executeLsCommand("/non-existent_dir_12345");

    // Test Case 4: A malicious string that should be caught by the validation.
    executeLsCommand("&& whoami");

    // Test Case 5: Another malicious string that should be caught by the validation.
    executeLsCommand(";/bin/bash");
    
    return 0;
}