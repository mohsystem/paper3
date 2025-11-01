#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cstdlib>

/**
 * Executes the 'id' command for a given username in a secure way.
 * This function forks and uses execvp to avoid shell injection.
 *
 * @param username The username to look up.
 * @return The exit code of the 'id' command, or a negative value on error.
 *         -1: Invalid username format.
 *         -2: Fork failed.
 *         -3: waitpid failed.
 *         -4: Child terminated abnormally.
 */
int executeIdCommand(const std::string& username) {
    // Regex for a safe, typical POSIX username.
    const std::regex username_pattern("^[a-zA-Z_][a-zA-Z0-9_.-]{0,31}$");
    if (!std::regex_match(username, username_pattern)) {
        std::cerr << "Error: Invalid username format for '" << username << "'." << std::endl;
        return -1; // Indicate invalid input
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork");
        return -2;
    } else if (pid == 0) {
        // Child process
        // Prepare arguments for execvp
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>("id"));
        argv.push_back(const_cast<char*>(username.c_str()));
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        
        // execvp only returns on error
        perror("execvp");
        _exit(127); // Standard exit code for command not found/executable error
    } else {
        // Parent process
        int status;
        // Wait for the child process to terminate
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return -3;
        }

        // Check if the child exited normally
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status); // Return the child's exit code
        } else {
            // Child terminated abnormally (e.g., by a signal)
            std::cerr << "Error: Child process terminated abnormally for username '" << username << "'." << std::endl;
            return -4;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // Handle command-line argument if provided
        std::cout << "--- Running with command-line argument ---" << std::endl;
        std::string username = argv[1];
        std::cout << "Running command for username: " << username << std::endl;
        int exitStatus = executeIdCommand(username);
        std::cout << "Exit Status: " << exitStatus << std::endl;
        std::cout << "----------------------------------------" << std::endl << std::endl;
    }

    // Run 5 hardcoded test cases
    std::cout << "--- Running 5 Hardcoded Test Cases ---" << std::endl;
    std::vector<std::string> testUsernames = {
        "root",                 // Test case 1: Valid, existing user
        "daemon",               // Test case 2: Another valid, existing user
        "nonexistentuser12345", // Test case 3: Valid format, but non-existent user
        "invalid;user",         // Test case 4: Invalid format (command injection attempt)
        " a bad user"           // Test case 5: Invalid format (contains spaces)
    };

    for (size_t i = 0; i < testUsernames.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ": Running command for username '" << testUsernames[i] << "'" << std::endl;
        int exitStatus = executeIdCommand(testUsernames[i]);
        std::cout << "Exit Status: " << exitStatus << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }

    return 0;
}