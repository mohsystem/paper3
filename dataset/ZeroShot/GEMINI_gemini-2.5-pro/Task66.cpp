#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <stdexcept>

// This implementation uses POSIX-specific APIs (fork, execvp, pipe).
// It will not compile on non-POSIX systems like Windows without a compatibility layer.
#ifdef __unix__
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#endif

class Task66 {
public:
    static std::string executeSecureCommand(const std::string& command) {
#ifndef __unix__
        return "Error: This implementation is for POSIX-compliant systems only.";
#else
        if (command.empty() || command.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
            return "Error: Command cannot be null or empty.";
        }

        // Tokenize the command string by whitespace. This is a simplification.
        std::istringstream iss(command);
        std::vector<std::string> parts;
        std::string part;
        while (iss >> part) {
            parts.push_back(part);
        }

        if (parts.empty()) {
            return "Error: Command is empty after parsing.";
        }

        // 1. A whitelist of allowed commands is the most effective security measure.
        const std::set<std::string> ALLOWED_COMMANDS = {"ls", "pwd", "date", "echo"};
        const std::string& baseCommand = parts[0];

        if (ALLOWED_COMMANDS.find(baseCommand) == ALLOWED_COMMANDS.end()) {
            return "Error: Command '" + baseCommand + "' is not allowed.";
        }

        // 2. Validate arguments for insecure patterns.
        for (size_t i = 1; i < parts.size(); ++i) {
            const std::string& arg = parts[i];
            if (arg.find("..") != std::string::npos || arg.find_first_of(";&|<>`$()\\{\\}") != std::string::npos) {
                return "Error: Argument '" + arg + "' contains forbidden characters or patterns.";
            }
        }

        // Prepare arguments for execvp: a null-terminated array of C-style strings
        std::vector<char*> argv;
        for (const auto& p : parts) {
            argv.push_back(const_cast<char*>(p.c_str()));
        }
        argv.push_back(nullptr);

        int pipefd[2];
        if (pipe(pipefd) == -1) { return "Error: Failed to create pipe."; }

        pid_t pid = fork();
        if (pid == -1) {
            close(pipefd[0]); close(pipefd[1]);
            return "Error: Failed to fork process.";
        }

        if (pid == 0) { // Child process
            close(pipefd[0]); // Close read end in child
            // Redirect stdout and stderr to the pipe
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);

            // 3. execvp replaces the process image and does not use a shell. This is secure.
            execvp(argv[0], argv.data());
            
            // If execvp returns, an error occurred
            perror("execvp");
            _exit(127); // Use _exit in child after fork
        } else { // Parent process
            close(pipefd[1]); // Close write end in parent

            std::string result;
            char buffer[4096];
            
            int status;
            pid_t wait_result = -1;
            // 4. Timeout mechanism (50 * 100ms = 5 seconds)
            for (int i = 0; i < 50; ++i) {
                wait_result = waitpid(pid, &status, WNOHANG);
                if (wait_result == pid) break; // Child has terminated
                usleep(100000); // Sleep for 100ms
            }

            // Read all available data from the pipe
            ssize_t count;
            while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
                result.append(buffer, count);
            }
            
            close(pipefd[0]);

            if (wait_result == 0) { // Child has not terminated, so it timed out
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0); // Clean up the zombie process
                return "Error: Command timed out.";
            }

            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                 return "Error: Command exited with code " + std::to_string(WEXITSTATUS(status)) + ".\nOutput:\n" + result;
            }

            return result;
        }
#endif
    }
};

int main() {
    std::vector<std::string> testCommands = {
        "ls -l",                // 1. Valid command
        "echo Hello World",     // 2. Valid command with arguments
        "whoami",               // 3. Invalid command (not in allowlist)
        "ls; rm -rf /",         // 4. Injection attempt (tokenized to 'ls;', which is rejected)
        "ls ../../"             // 5. Path traversal attempt
    };

    for (const auto& cmd : testCommands) {
        std::cout << "Executing: '" << cmd << "'" << std::endl;
        std::cout << "-------------------------" << std::endl;
        std::string result = Task66::executeSecureCommand(cmd);
        std::cout << "Result:\n" << result << std::endl;
        std::cout << "=========================" << std::endl;
    }

    return 0;
}