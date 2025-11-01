
#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include <array>
#include <memory>
#include <sys/wait.h>
#include <unistd.h>

// Whitelist pattern: alphanumeric, underscore, hyphen only
// This prevents command injection through special shell characters
const std::regex SAFE_USERNAME_PATTERN("^[a-zA-Z0-9_-]+$");
const size_t MAX_USERNAME_LENGTH = 32;

/**
 * Executes the id command for a given username.
 * Returns the exit status of the command.
 * 
 * @param username The username to query (must match safe pattern)
 * @return Exit status: 0 for success, non-zero for failure, -1 for invalid input
 */
int executeIdCommand(const std::string& username) {
    // Input validation: check for empty string
    if (username.empty()) {
        std::cerr << "Error: Username cannot be empty" << std::endl;
        return -1;
    }

    // Input validation: check length to prevent buffer issues
    if (username.length() > MAX_USERNAME_LENGTH) {
        std::cerr << "Error: Username exceeds maximum length" << std::endl;
        return -1;
    }

    // Input validation: sanitize username to prevent command injection
    // Only allow alphanumeric characters, underscore, and hyphen
    if (!std::regex_match(username, SAFE_USERNAME_PATTERN)) {
        std::cerr << "Error: Username contains invalid characters" << std::endl;
        return -1;
    }

    // Use fork and execvp to avoid shell interpretation
    // This prevents command injection attacks
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        std::cerr << "Error: Failed to create process" << std::endl;
        return -1;
    }

    if (pid == 0) {
        // Child process
        // Build argument array - execvp does not use shell
        const char* args[] = {
            "id",
            username.c_str(),
            nullptr  // NULL terminator required
        };

        // Execute id command directly without shell
        // execvp searches PATH and does not invoke shell
        execvp("id", const_cast<char* const*>(args));

        // If execvp returns, it failed
        std::cerr << "Error: Failed to execute command" << std::endl;
        _exit(127);  // Use _exit in child process
    }

    // Parent process
    int status;
    pid_t result = waitpid(pid, &status, 0);

    if (result == -1) {
        std::cerr << "Error: Failed to wait for process" << std::endl;
        return -1;
    }

    // Extract exit status
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        // Process terminated by signal
        std::cerr << "Error: Process terminated by signal" << std::endl;
        return -1;
    }

    return -1;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::array<std::string, 5> testUsers = {
        "root", "nobody", "testuser123", "user-name", "invalid@user"
    };

    if (argc > 1) {
        // Use command line argument if provided
        std::string username(argv[1]);
        int exitStatus = executeIdCommand(username);
        std::cout << "Exit status: " << exitStatus << std::endl;
    } else {
        // Run test cases
        std::cout << "Running test cases:" << std::endl;
        for (const auto& user : testUsers) {
            std::cout << "\\n--- Testing with username: " << user << " ---" << std::endl;
            int exitStatus = executeIdCommand(user);
            std::cout << "Exit status: " << exitStatus << std::endl;
        }
    }

    return 0;
}
