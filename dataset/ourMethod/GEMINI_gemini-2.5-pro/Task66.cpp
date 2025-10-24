#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <stdexcept>
#include <memory>
#include <array>

// POSIX-specific includes for process management
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

// A set of allowed commands to prevent execution of arbitrary programs.
const std::set<std::string> ALLOWED_COMMANDS = {"ls", "echo", "date", "whoami"};

/**
 * Validates and executes a shell command securely using fork/exec.
 * NOTE: This implementation is for POSIX-compliant systems (Linux, macOS).
 *
 * @param command The command string to execute.
 * @return The combined standard output and standard error of the command.
 */
std::string executeCommand(const std::string& command) {
    if (command.empty() || command.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
        return "Error: Command cannot be null or empty.";
    }

    // Disallow shell metacharacters as a primary security measure.
    if (command.find_first_of(";&|<>`$()") != std::string::npos) {
        return "Error: Command contains forbidden shell metacharacters.";
    }
    
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.empty()) {
        return "Error: Command is empty.";
    }

    if (ALLOWED_COMMANDS.find(tokens[0]) == ALLOWED_COMMANDS.end()) {
        return "Error: Command '" + tokens[0] + "' is not allowed.";
    }

    // Prepare arguments for execvp. It requires a null-terminated array of char pointers.
    std::vector<char*> argv;
    for (const auto& s : tokens) {
        argv.push_back(const_cast<char*>(s.c_str()));
    }
    argv.push_back(nullptr);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "Error: Failed to create pipe.";
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "Error: Failed to fork process.";
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close read end of the pipe
        // Redirect stdout and stderr to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        execvp(argv[0], argv.data());
        // If execvp returns, an error occurred.
        perror("execvp");
        _exit(127); // Use _exit in child to avoid flushing parent's buffers
    } else { // Parent process
        close(pipefd[1]); // Close write end of the pipe

        std::string result;
        std::array<char, 256> buffer;
        ssize_t count;
        while ((count = read(pipefd[0], buffer.data(), buffer.size())) > 0) {
            result.append(buffer.data(), count);
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        result += "\nExited with code: ";
        if (WIFEXITED(status)) {
            result += std::to_string(WEXITSTATUS(status));
        } else {
            result += " (abnormal termination)";
        }
        return result;
    }
}

int main() {
    std::vector<std::string> testCommands = {
        "ls -l",
        "echo hello from cpp",
        "grep root /etc/passwd", // Not in allowlist
        "ls; whoami",            // Contains metacharacter
        ""                       // Empty command
    };

    for (const auto& cmd : testCommands) {
        std::cout << "Executing command: \"" << cmd << "\"" << std::endl;
        std::string result = executeCommand(cmd);
        std::cout << "Result:\n" << result << std::endl;
        std::cout << "------------------------------------" << std::endl;
    }

    return 0;
}