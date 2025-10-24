#include <iostream>
#include <string>
#include <vector>
#include <regex>

// The following headers are for the POSIX API (fork, exec, waitpid).
// This code will only compile and run on POSIX-compliant systems like Linux and macOS.
#include <unistd.h>
#include <sys/wait.h>

// Note: A secure implementation for Windows would require the Windows-specific
// CreateProcess() API instead of fork/exec.

class Task158 {
public:
    /**
     * Validates that the input contains only characters that are safe for a hostname or IP address.
     * This is a critical defense-in-depth measure that uses an "allow-list" approach
     * to prevent characters that could be used for command injection (e.g., ';', '|', '&', '$').
     * @param input The string to validate.
     * @return true if the input is valid, false otherwise.
     */
    static bool isValidInput(const std::string& input) {
        if (input.empty()) {
            return false;
        }
        // Allow alphanumeric characters, dots, and hyphens.
        static const std::regex pattern("^[a-zA-Z0-9.-]+$");
        return std::regex_match(input, pattern);
    }

    /**
     * Executes an OS command (ping) securely using user-provided input.
     * It uses the POSIX fork/exec model, which passes arguments separately and
     * is the standard, secure way to avoid command injection on UNIX-like systems.
     * @param userInput The destination (hostname or IP) to be pinged.
     */
    static void secureOsCommand(const std::string& userInput) {
        std::cout << "--- Executing command for input: '" << userInput << "' ---" << std::endl;

        if (!isValidInput(userInput)) {
            std::cout << "Invalid input format. Aborting command execution." << std::endl;
            std::cout << "-------------------------------------------------" << std::endl;
            return;
        }

        pid_t pid = fork();

        if (pid == -1) {
            // Fork failed
            perror("fork");
        } else if (pid == 0) {
            // --- This is the child process ---
            // execlp replaces the current process with a new one (ping).
            // Arguments are passed as a list of separate strings, preventing shell interpretation.
            // The list must be terminated by a NULL pointer.
            execlp("ping", "ping", "-c", "4", userInput.c_str(), (char *)nullptr);

            // If execlp returns, it means an error occurred.
            perror("execlp");
            _exit(EXIT_FAILURE); // Use _exit in child to prevent flushing parent's stdio buffers
        } else {
            // --- This is the parent process ---
            int status;
            // Wait for the child process to terminate
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                std::cout << "\nExited with code: " << WEXITSTATUS(status) << std::endl;
            } else {
                std::cout << "\nChild process terminated abnormally." << std::endl;
            }
        }
        std::cout << "-------------------------------------------------" << std::endl;
    }
};

int main() {
    std::vector<std::string> testCases = {
        "127.0.0.1",              // Valid IP address
        "google.com",             // Valid hostname
        "invalid-hostname!",      // Invalid character, should be rejected by validation
        "127.0.0.1; ls",          // Classic command injection, should be rejected by validation
        "$(reboot)"               // Command substitution, should be rejected by validation
    };

    for (const auto& testCase : testCases) {
        Task158::secureOsCommand(testCase);
    }

    return 0;
}