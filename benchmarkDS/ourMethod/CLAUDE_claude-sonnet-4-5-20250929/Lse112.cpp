
#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include <array>
#include <memory>
#include <unistd.h>
#include <sys/wait.h>

// Maximum allowed hostname length per RFC 1035
const size_t MAX_HOST_LENGTH = 253;
// Maximum output buffer size to prevent memory exhaustion
const size_t MAX_OUTPUT_SIZE = 65536;

// Validates hostname format to prevent command injection
// Only allows alphanumeric characters, dots, and hyphens
bool isValidHost(const std::string& host) {
    // Check length to prevent buffer issues
    if (host.empty() || host.length() > MAX_HOST_LENGTH) {
        return false;
    }
    
    // Whitelist pattern: reject shell metacharacters
    // This prevents injection of commands like ; | & $ ( ) ` etc.
    std::regex validPattern("^[a-zA-Z0-9.-]+$");
    return std::regex_match(host, validPattern);
}

// Executes ping command safely without shell involvement
// Returns ping output or error message
std::string executePing(const std::string& host) {
    int pipefd[2];
    
    // Create pipe for reading command output
    if (pipe(pipefd) == -1) {
        return "Error: Pipe creation failed";
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "Error: Fork failed";
    }
    
    if (pid == 0) {
        // Child process: execute ping
        close(pipefd[0]); // Close read end
        
        // Redirect stdout and stderr to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        // Execute ping with separate arguments - no shell involved
        // This prevents command injection as execl does not invoke a shell
        // Arguments are passed directly to the ping binary
        execl("/bin/ping", "ping", "-c", "4", host.c_str(), (char*)NULL);
        
        // If execl fails, exit child process
        _exit(1);
    } else {
        // Parent process: read output
        close(pipefd[1]); // Close write end
        
        std::string output = "Ping result for " + host + ":\\n";
        std::array<char, 4096> buffer;
        ssize_t bytesRead;
        size_t totalBytes = 0;
        
        // Read output with size limit to prevent memory exhaustion
        while ((bytesRead = read(pipefd[0], buffer.data(), buffer.size())) > 0) {
            totalBytes += bytesRead;
            if (totalBytes > MAX_OUTPUT_SIZE) {
                close(pipefd[0]);
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
                return "Error: Output too large";
            }
            output.append(buffer.data(), bytesRead);
        }
        
        close(pipefd[0]);
        
        // Wait for child process to complete
        int status;
        waitpid(pid, &status, 0);
        
        return output;
    }
}

// Simulated ping handler for demonstration
// In a real web framework, this would be bound to a URL route
std::string pingHandler(const std::string& url) {
    if (url.empty()) {
        // Input validation: reject empty input
        return "Error: Missing 'url' parameter";
    }
    
    if (!isValidHost(url)) {
        // Input validation: reject invalid hostnames to prevent command injection
        return "Error: Invalid URL format";
    }
    
    // Execute ping with validated input
    return executePing(url);
}

int main() {
    std::cout << "Ping service simulation\\n";
    std::cout << "Test cases:\\n\\n";
    
    // Test case 1: Valid hostname
    std::cout << "Test 1 - localhost:\\n";
    std::cout << pingHandler("localhost") << "\\n\\n";
    
    // Test case 2: Valid IP address
    std::cout << "Test 2 - 127.0.0.1:\\n";
    std::cout << pingHandler("127.0.0.1") << "\\n\\n";
    
    // Test case 3: Valid domain
    std::cout << "Test 3 - google.com:\\n";
    std::cout << pingHandler("google.com") << "\\n\\n";
    
    // Test case 4: Empty input
    std::cout << "Test 4 - empty string:\\n";
    std::cout << pingHandler("") << "\\n\\n";
    
    // Test case 5: Command injection attempt (blocked)
    std::cout << "Test 5 - injection attempt (localhost;ls):\\n";
    std::cout << pingHandler("localhost;ls") << "\\n\\n";
    
    return 0;
}
