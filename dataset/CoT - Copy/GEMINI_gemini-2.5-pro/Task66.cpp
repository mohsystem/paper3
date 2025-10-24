#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <stdexcept>
#include <memory>

// Required for POSIX-specific process management
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// 1. Validation: Use an allowlist of safe commands.
const std::set<std::string> ALLOWED_COMMANDS = {"ls", "echo", "date", "pwd", "whoami"};

/**
 * Splits a string by whitespace into a vector of strings.
 * This is a helper for secure parsing.
 * @param str The string to split.
 * @return A vector of tokens.
 */
std::vector<std::string> splitString(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * Validates, executes a shell command, and returns its output.
 * This implementation is for POSIX-compliant systems (Linux, macOS).
 * It uses fork() and execvp() to avoid shell command injection.
 *
 * @param command The command string to execute.
 * @return The standard output and standard error from the command, or an error message.
 */
std::string executeCommand(const std::string& command) {
    if (command.empty() || command.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
        return "Error: Command cannot be null or empty.";
    }

    // 2. Secure Parsing
    std::vector<std::string> command_parts = splitString(command);
    if (command_parts.empty()) {
        return "Error: Command is empty after parsing.";
    }

    const std::string& base_command = command_parts[0];

    // 3. Validation against Allowlist
    if (ALLOWED_COMMANDS.find(base_command) == ALLOWED_COMMANDS.end()) {
        return "Error: Command '" + base_command + "' is not allowed.";
    }

    // 4. Secure Execution: Prepare for fork/exec
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        return "Error: Failed to create pipe.";
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return "Error: Failed to fork process.";
    }

    if (pid == 0) { // Child process
        close(pipe_fd[0]); // Close read end
        // Redirect stdout and stderr to the pipe
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]); // Close original write end

        // Convert vector<string> to char* const[] for execvp
        std::vector<char*> argv;
        for (const auto& part : command_parts) {
            argv.push_back(const_cast<char*>(part.c_str()));
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        // If execvp returns, it must have failed.
        perror("execvp");
        _exit(127); // Use _exit in child after fork
    } else { // Parent process
        close(pipe_fd[1]); // Close write end

        std::string output;
        char buffer[256];
        ssize_t bytes_read;
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytes_read);
        }
        close(pipe_fd[0]);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            output += "\nCommand exited with non-zero status: " + std::to_string(WEXITSTATUS(status));
        }

        return output;
    }
}

class Task66 {
public:
    static void main() {
        // --- Test Cases ---

        // Test Case 1: Valid and allowed command
        std::cout << "--- Test Case 1: ls -l ---" << std::endl;
        std::string test1 = "ls -l";
        std::cout << "Executing: " << test1 << std::endl;
        std::cout << "Result:\n" << executeCommand(test1) << std::endl;

        // Test Case 2: Valid command with arguments
        std::cout << "--- Test Case 2: echo Hello World ---" << std::endl;
        std::string test2 = "echo Hello Secure World";
        std::cout << "Executing: " << test2 << std::endl;
        std::cout << "Result:\n" << executeCommand(test2) << std::endl;

        // Test Case 3: Disallowed command (potential for harm)
        std::cout << "--- Test Case 3: rm -rf / ---" << std::endl;
        std::string test3 = "rm -rf /";
        std::cout << "Executing: " << test3 << std::endl;
        std::cout << "Result:\n" << executeCommand(test3) << std::endl;
        
        // Test Case 4: Command with shell metacharacters (attempted injection)
        std::cout << "--- Test Case 4: ls; whoami ---" << std::endl;
        std::string test4 = "ls; whoami"; // Will be parsed as `ls;` and `whoami`
        std::cout << "Executing: " << test4 << std::endl;
        std::cout << "Result:\n" << executeCommand(test4) << std::endl;

        // Test Case 5: Empty command
        std::cout << "--- Test Case 5: Empty Command ---" << std::endl;
        std::string test5 = " ";
        std::cout << "Executing: \" \"" << std::endl;
        std::cout << "Result:\n" << executeCommand(test5) << std::endl;
    }
};

int main() {
    Task66::main();
    return 0;
}