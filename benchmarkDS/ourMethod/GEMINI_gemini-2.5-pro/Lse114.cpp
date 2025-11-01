#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // For system()
#include <cstdio>  // For snprintf

/**
 * Validates an argument string by checking for forbidden shell metacharacters.
 * @param arg The string to validate.
 * @return true if the string is safe, false otherwise.
 */
bool is_argument_safe(const std::string& arg) {
    // Rule #1: Validate input against a blacklist of shell metacharacters.
    // A whitelist approach is generally safer but more restrictive.
    const std::string forbidden_chars = ";&|`'\"$*<>(){}\\";
    return arg.find_first_of(forbidden_chars) == std::string::npos;
}

/**
 * Constructs and executes an 'ls' command using the system() call.
 *
 * @param path The directory path to list.
 * @param flags The flags to pass to the 'ls' command (e.g., "-l").
 */
void executeLsCommand(const std::string& path, const std::string& flags) {
    if (!is_argument_safe(path) || !is_argument_safe(flags)) {
        std::cerr << "Error: Potentially unsafe characters detected in input. Aborting." << std::endl;
        return;
    }

    // Rule #3: Ensure buffer operations adhere to boundaries.
    // Calculate size needed for "ls " + flags + " " + path + null terminator
    size_t required_size = 3 + flags.length() + 1 + path.length() + 1;
    
    // Use a std::vector as a dynamically-sized buffer.
    std::vector<char> command_buffer(required_size);
    
    // The "print function" (snprintf) is used to create the string for the system function.
    // snprintf is safe against buffer overflows as it takes the buffer size as an argument.
    snprintf(command_buffer.data(), command_buffer.size(), "ls %s %s", flags.c_str(), path.c_str());
    
    std::string command(command_buffer.data());

    std::cout << "Executing command: " << command << std::endl;
    std::cout << "---" << std::endl;

    // Rule #2: Use potentially dangerous functions with caution.
    // system() invokes a shell, which is a security risk if the command string
    // is not constructed from validated and sanitized inputs.
    int return_code = std::system(command.c_str());

    std::cout << "---" << std::endl;
    // The return code format is implementation-defined, but this shows the raw value.
    std::cout << "Command finished with exit code: " << return_code << std::endl;
}

int main() {
    std::cout << "--- Test Case 1: List current directory ---" << std::endl;
    executeLsCommand(".", "");

    std::cout << "\n--- Test Case 2: List /tmp with long format ---" << std::endl;
    executeLsCommand("/tmp", "-l");

    std::cout << "\n--- Test Case 3: Non-existent directory ---" << std::endl;
    executeLsCommand("/non_existent_dir_12345", "-a");

    std::cout << "\n--- Test Case 4: Invalid path (command injection attempt) ---" << std::endl;
    executeLsCommand(".; ls", "-l");

    std::cout << "\n--- Test Case 5: Invalid flag (command injection attempt) ---" << std::endl;
    executeLsCommand(".", "-l; echo vulnerable");

    return 0;
}