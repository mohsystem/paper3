#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // For system()
#include <sstream> // For stringstream

// Allowed characters in arguments. A whitelist approach is safer.
const std::string ALLOWED_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_./*?";

/**
 * @brief Validates an argument string to ensure it contains only allowed characters.
 * 
 * @param arg The argument to validate.
 * @return true if the argument is safe, false otherwise.
 */
bool is_valid_argument(const std::string& arg) {
    // SECURITY: Ensure argument doesn't contain forbidden characters that could
    // be interpreted by the shell. An empty argument is valid (will be skipped).
    if (arg.empty()) {
        return true;
    }
    // find_first_not_of returns std::string::npos if all characters are in ALLOWED_CHARS.
    return arg.find_first_not_of(ALLOWED_CHARS) == std::string::npos;
}

/**
 * @brief Executes the 'ls' command with the given arguments.
 * 
 * This function validates each argument to prevent command injection before
 * constructing the command string and passing it to std::system.
 * 
 * Note: Using non-shell-based APIs (like fork/exec on POSIX or CreateProcess on
 * Windows) is generally safer. This implementation uses std::system to match
 * the prompt's requirements.
 * 
 * @param args A vector of strings representing the arguments for 'ls'.
 */
void executeLsCommand(const std::vector<std::string>& args) {
    std::stringstream command_stream;
    command_stream << "ls";

    for (const auto& arg : args) {
        if (arg.empty()) {
            continue;
        }
        // SECURITY: Validate each argument before use.
        if (!is_valid_argument(arg)) {
            std::cerr << "Error: Invalid or potentially malicious argument detected: " << arg << std::endl;
            return;
        }
        // It's good practice to quote arguments, though our whitelist makes it less critical.
        command_stream << " '" << arg << "'";
    }

    // The 'print function' (stringstream) is used to create the string.
    std::string command = command_stream.str();
    std::cout << "Executing command: " << command << std::endl;

    // The system function executes the command.
    int return_code = std::system(command.c_str());

    std::cout << "system() returned: " << return_code << std::endl;
}

int main() {
    std::cout << "--- Test Case 1: ls current directory ---" << std::endl;
    executeLsCommand({});

    std::cout << "\n--- Test Case 2: ls -l -a ---" << std::endl;
    executeLsCommand({"-l", "-a"});

    std::cout << "\n--- Test Case 3: ls specific path ---" << std::endl;
    executeLsCommand({"."});

    std::cout << "\n--- Test Case 4: ls non-existent file (shows error stream) ---" << std::endl;
    executeLsCommand({"/non_existent_dir_12345"});
    
    std::cout << "\n--- Test Case 5: Attempted command injection (should be rejected) ---" << std::endl;
    executeLsCommand({"-l;id"});

    return 0;
}