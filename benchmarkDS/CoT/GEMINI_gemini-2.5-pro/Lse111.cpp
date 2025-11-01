#include <iostream>
#include <string>
#include <vector>
#include <regex>

// The following implementation is for POSIX-compliant systems (like Linux, macOS).
// It will not compile or run on Windows without modification to use the Windows API (e.g., CreateProcess).
#ifdef _WIN32
std::string ping(const std::string& url) {
    return "Error: This ping implementation is for POSIX systems (Linux, macOS) only.";
}
#else
#include <unistd.h>
#include <sys/wait.h>
#include <cstdio> // For perror

/**
 * Executes the system's ping command for a given URL or IP address.
 * To prevent command injection, this function validates the input and uses
 * fork() and execvp() to execute the command directly, bypassing the shell.
 * Output is captured from the child process's stdout/stderr via a pipe.
 *
 * @param url The hostname or IP address to ping.
 * @return The output of the ping command, or an error message.
 */
std::string ping(const std::string& url) {
    // Security: Validate input to prevent command injection.
    // This regex allows for hostnames and IP addresses but disallows shell metacharacters.
    if (!std::regex_match(url, std::regex("^[a-zA-Z0-9\\.\\-]+$")) || url.length() > 255) {
        return "Error: Invalid or potentially malicious hostname provided.";
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "Error: Failed to create pipe.";
    }

    pid_t pid = fork();
    if (pid == -1) {
        return "Error: Failed to fork process.";
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close read end of the pipe
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the pipe
        dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to the pipe
        close(pipefd[1]); // Close the now-duplicated write end

        // Prepare arguments for execvp. This is the secure alternative to system().
        std::vector<char*> args;
        args.push_back(const_cast<char*>("ping"));
        args.push_back(const_cast<char*>("-c"));
        args.push_back(const_cast<char*>("4"));
        args.push_back(const_cast<char*>(url.c_str()));
        args.push_back(nullptr);

        execvp(args[0], args.data());
        // If execvp returns, it must have failed.
        perror("execvp failed");
        _exit(127); 
    } else { // Parent process
        close(pipefd[1]); // Close write end of the pipe

        std::string result;
        char buffer[256];
        ssize_t count;
        while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            result += buffer;
        }
        close(pipefd[0]); // Close read end

        waitpid(pid, nullptr, 0); // Wait for child to finish
        
        return result;
    }
}
#endif

class Lse111 {
public:
    static void runTests() {
        std::vector<std::string> testCases = {
            "8.8.8.8",                  // Test case 1: Valid IP
            "google.com",               // Test case 2: Valid domain
            "localhost",                // Test case 3: Local host
            "non-existent-domain.xyz",  // Test case 4: Non-existent but syntactically valid hostname
            "8.8.8.8; ls -la"           // Test case 5: Malicious input (should be rejected by validation)
        };

        for (size_t i = 0; i < testCases.size(); ++i) {
            std::cout << "--- Test Case " << (i + 1) << ": Pinging '" << testCases[i] << "' ---" << std::endl;
            std::string result = ping(testCases[i]);
            std::cout << result << std::endl;
            std::cout << "--- End of Test Case " << (i + 1) << " ---\n" << std::endl;
        }
    }
};

int main() {
    Lse111::runTests();
    return 0;
}