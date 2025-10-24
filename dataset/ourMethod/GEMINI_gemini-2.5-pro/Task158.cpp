#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <array>
#include <set>
#include <cstdlib>

// A whitelist of allowed directory names.
const std::set<std::string> ALLOWED_DIRECTORIES = {"safe_dir_1", "safe_dir_2", "."};

/**
 * Executes a command with arguments safely, avoiding shell injection.
 * This implementation is for POSIX-compliant systems (e.g., Linux, macOS).
 *
 * @param directory The directory name to list. Must be in the allowed list.
 * @return The stdout of the command.
 * @throws std::runtime_error on failure.
 */
std::string listDirectoryContents(const std::string& directory) {
    if (ALLOWED_DIRECTORIES.find(directory) == ALLOWED_DIRECTORIES.end()) {
        return "Error: Invalid or disallowed directory specified.";
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        throw std::runtime_error("Error: Failed to create pipe.");
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        throw std::runtime_error("Error: Failed to fork process.");
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe as well
        close(pipefd[1]);

        // Arguments for execvp. The user input is passed as a single argument.
        // The list must be null-terminated. This avoids shell interpretation.
        const char* cmd = "ls";
        char* args[] = {const_cast<char*>(cmd), const_cast<char*>("-l"), const_cast<char*>(directory.c_str()), nullptr};

        execvp(cmd, args);

        // execvp only returns on error
        perror("execvp");
        _exit(127); // Exit child with an error code
    } else { // Parent process
        close(pipefd[1]); // Close unused write end

        std::string result;
        std::array<char, 256> buffer;
        ssize_t count;
        while ((count = read(pipefd[0], buffer.data(), buffer.size())) > 0) {
            result.append(buffer.data(), count);
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            return "Error: Command failed to execute properly.\n" + result;
        }

        return result;
    }
}

int main() {
    // Note: Using system() here is only for test setup. The core function
    // avoids it for security.
    system("mkdir -p safe_dir_1");
    system("mkdir -p safe_dir_2");
    
    std::vector<std::string> testCases = {
        ".",
        "safe_dir_1",
        "safe_dir_2",
        "non_existent_dir",
        "safe_dir_1; whoami"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": Input='" << testCases[i] << "' ---" << std::endl;
        try {
            std::string result = listDirectoryContents(testCases[i]);
            std::cout << result << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
        std::cout << "--- End Test Case " << (i + 1) << " ---\n" << std::endl;
    }

    return 0;
}