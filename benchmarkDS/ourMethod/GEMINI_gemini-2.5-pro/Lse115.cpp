#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>
#include <memory>
#include <regex>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

// A pattern to validate arguments. It allows alphanumeric characters, dashes, dots, and slashes.
// This is a restrictive whitelist to prevent command injection.
// It does not prevent path traversal (e.g., "ls ../../"), which could be an information disclosure vulnerability.
const std::regex SAFE_ARG_PATTERN("^[a-zA-Z0-9_./-]+$");

/**
 * Creates a command string using a format string and arguments.
 * Uses vsnprintf to prevent buffer overflows.
 *
 * @param format The format string.
 * @param ...    The arguments to be formatted into the string.
 * @return The formatted command string.
 */
std::string createCommandString(const char* format, ...) {
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);
    
    // Determine required size using a safe call to vsnprintf
    int size = vsnprintf(nullptr, 0, format, args1);
    va_end(args1);
    if (size < 0) {
        va_end(args2);
        return ""; // Encoding error
    }
    
    std::unique_ptr<char[]> buffer(new char[size + 1]);
    if (vsnprintf(buffer.get(), size + 1, format, args2) < 0) {
        va_end(args2);
        return ""; // Encoding error during formatting
    }
    va_end(args2);
    
    return std::string(buffer.get());
}

/**
 * Executes an 'ls' command provided as a single string.
 * The command string is parsed and validated before execution to prevent
 * command injection vulnerabilities. It specifically avoids using a shell.
 *
 * @param command The command string to execute, e.g., "ls -l /tmp".
 */
void executeLsCommand(const std::string& command) {
    if (command.empty()) {
        std::cerr << "Error: Command string is empty." << std::endl;
        return;
    }

    // Simple parsing by splitting on whitespace. This does not handle arguments with spaces.
    std::stringstream ss(command);
    std::string item;
    std::vector<std::string> parts;
    while (ss >> item) {
        parts.push_back(item);
    }

    if (parts.empty()) {
        std::cerr << "Error: Invalid command format." << std::endl;
        return;
    }

    // Security Check 1: The command must be exactly 'ls'.
    if (parts[0] != "ls") {
        std::cerr << "Error: Only 'ls' command is allowed. Found: " << parts[0] << std::endl;
        return;
    }

    // Security Check 2: Validate all arguments against a safe pattern.
    for (size_t i = 1; i < parts.size(); ++i) {
        if (!std::regex_match(parts[i], SAFE_ARG_PATTERN)) {
            std::cerr << "Error: Argument contains invalid characters: " << parts[i] << std::endl;
            return;
        }
    }

    // Convert std::vector<std::string> to char* const[] for execvp.
    std::vector<char*> argv;
    for (const auto& part : parts) {
        argv.push_back(const_cast<char*>(part.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return;
    }

    if (pid == 0) { // Child process
        // execvp replaces the child process image. It does not use a shell.
        execvp(argv[0], argv.data());
        // If execvp returns, an error occurred.
        perror("execvp failed");
        _exit(EXIT_FAILURE); // Use _exit in child after fork
    } else { // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        } else {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                 std::cerr << "'ls' command exited with code: " << WEXITSTATUS(status) << std::endl;
            }
        }
    }
}

int main() {
    std::cout << "--- Test Case 1: Simple 'ls .' ---" << std::endl;
    std::string cmd1 = createCommandString("%s %s", "ls", ".");
    std::cout << "Executing: " << cmd1 << std::endl;
    executeLsCommand(cmd1);
    std::cout << std::endl;

    std::cout << "--- Test Case 2: 'ls -l' with an option ---" << std::endl;
    std::string cmd2 = createCommandString("%s %s", "ls", "-l");
    std::cout << "Executing: " << cmd2 << std::endl;
    executeLsCommand(cmd2);
    std::cout << std::endl;
    
    std::cout << "--- Test Case 3: 'ls' on a non-existent directory ---" << std::endl;
    std::string cmd3 = createCommandString("%s %s", "ls", "non_existent_dir");
    std::cout << "Executing: " << cmd3 << std::endl;
    executeLsCommand(cmd3);
    std::cout << std::endl;

    std::cout << "--- Test Case 4: Malicious command injection attempt (should be blocked) ---" << std::endl;
    std::string cmd4 = createCommandString("%s %s", "ls", ".;whoami");
    std::cout << "Executing: " << cmd4 << std::endl;
    executeLsCommand(cmd4);
    std::cout << std::endl;

    std::cout << "--- Test Case 5: Different command attempt (should be blocked) ---" << std::endl;
    std::string cmd5 = createCommandString("%s", "whoami");
    std::cout << "Executing: " << cmd5 << std::endl;
    executeLsCommand(cmd5);
    std::cout << std::endl;

    return 0;
}