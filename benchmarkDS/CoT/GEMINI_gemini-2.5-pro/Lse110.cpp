#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>    // For fork, execvp
#include <sys/wait.h>  // For waitpid
#include <cstdlib>     // For exit
#include <cstring>     // For strerror
#include <cerrno>      // For errno

/**
 * Executes the 'id <username>' command securely using fork and execvp.
 * This method avoids the shell, thus preventing command injection vulnerabilities.
 *
 * @param username The username to look up.
 * @return The exit status of the 'id' command. Returns 0 on success, a positive
 *         integer on command failure (e.g., user not found), and -1 on a
 *         system-level error (e.g., fork failed).
 */
int runIdCommand(const std::string& username) {
    if (username.empty()) {
        std::cerr << "Error: Username cannot be empty." << std::endl;
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        std::cerr << "Failed to fork process: " << strerror(errno) << std::endl;
        return -1;
    } else if (pid == 0) {
        // This is the child process
        // Prepare arguments for execvp. The list must be null-terminated.
        // The first argument is the command itself.
        char* argv[] = {
            (char*)"id",
            (char*)username.c_str(),
            nullptr
        };

        // execvp replaces the current child process with the 'id' command.
        // It searches for 'id' in the system's PATH.
        execvp(argv[0], argv);

        // If execvp returns, an error has occurred.
        std::cerr << "Execvp failed: " << strerror(errno) << std::endl;
        exit(127); // Exit child with a specific error code
    } else {
        // This is the parent process
        int status;
        // Wait for the child process to terminate
        if (waitpid(pid, &status, 0) == -1) {
            std::cerr << "waitpid failed: " << strerror(errno) << std::endl;
            return -1;
        }

        // Check if the child terminated normally
        if (WIFEXITED(status)) {
            // Return the child's exit status
            return WEXITSTATUS(status);
        } else {
            // The child was terminated by a signal or other means
            std::cerr << "Child process did not terminate normally." << std::endl;
            return -1;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // If a command-line argument is provided, use it.
        std::string username = argv[1];
        std::cout << "Running command: id " << username << std::endl;
        int exitStatus = runIdCommand(username);
        std::cout << "Exit Status: " << exitStatus << std::endl;
    } else {
        // If no arguments are provided, run test cases.
        std::cout << "No command-line argument provided. Running test cases..." << std::endl;
        std::vector<std::string> testUsernames = {
            "root",                         // 1. Should succeed (exit 0)
            "nobody",                       // 2. Should succeed (exit 0)
            "non_existent_user_12345",      // 3. Should fail (exit non-zero)
            "root;whoami",                  // 4. Malicious input; should fail
            ""                              // 5. Empty string; should fail
        };

        for (const auto& user : testUsernames) {
            std::cout << "\n------------------------------------" << std::endl;
            std::cout << "Testing with username: \"" << user << "\"" << std::endl;
            int exitStatus = runIdCommand(user);
            std::cout << "Exit Status: " << exitStatus << std::endl;
            std::cout << "------------------------------------" << std::endl;
        }
    }
    return 0;
}