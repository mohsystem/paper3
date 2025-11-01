
// C++ implementation
// SECURITY NOTE: This demonstrates the UNSAFE pattern from the prompt.
// system() and popen() are inherently dangerous with user input (CWE-78).
// This implementation uses execvp with fork for safe command execution.

#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <array>

// Secure command execution with strict validation
std::string executeCommand(const std::string& directory) {
    // Input validation: reject empty strings
    if (directory.empty()) {
        return "Error: Invalid directory parameter";
    }
    
    // Length check: prevent excessive input
    if (directory.length() > 255) {
        return "Error: Directory name too long";
    }
    
    // Sanitization: block shell metacharacters and path traversal
    std::regex dangerous_chars(R"([;&|`$\\\\"'<>(){}[\\]*?~!#%^])");
    if (std::regex_search(directory, dangerous_chars) || 
        directory.find("..") != std::string::npos ||
        directory[0] == '/') {
        return "Error: Invalid characters in directory path";
    }
    
    // Whitelist validation: only alphanumeric, hyphens, underscores
    std::regex valid_pattern("^[a-zA-Z0-9_-]+$");
    if (!std::regex_match(directory, valid_pattern)) {
        return "Error: Directory name contains invalid characters";
    }
    
    // Create pipe for reading command output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return "Error: Failed to create pipe";
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "Error: Fork failed";
    }
    
    if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close read end
        
        // Redirect stdout to pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(1);
        }
        close(pipefd[1]);
        
        // Execute ls with argument array (NOT shell)
        // This prevents command injection
        const char* args[] = {"ls", "-la", directory.c_str(), nullptr};
        execvp("ls", const_cast<char* const*>(args));
        
        // If execvp returns, it failed
        _exit(1);
    }
    
    // Parent process
    close(pipefd[1]); // Close write end
    
    std::string output;
    std::array<char, 256> buffer;
    ssize_t count;
    
    // Read output from pipe
    while ((count = read(pipefd[0], buffer.data(), buffer.size() - 1)) > 0) {
        buffer[count] = '\\0';
        output += buffer.data();
    }
    
    close(pipefd[0]);
    
    // Wait for child process
    int status;
    waitpid(pid, &status, 0);
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return "Error: Command execution failed";
    }
    
    return output;
}

int main() {
    std::cout << "Test 1 - Valid directory:" << std::endl;
    std::cout << executeCommand("testdir") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 2 - Valid directory with underscore:" << std::endl;
    std::cout << executeCommand("my_folder") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 3 - Invalid: path traversal attempt:" << std::endl;
    std::cout << executeCommand("../etc") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 4 - Invalid: command injection attempt:" << std::endl;
    std::cout << executeCommand("test; rm -rf /") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 5 - Invalid: pipe attempt:" << std::endl;
    std::cout << executeCommand("test | cat /etc/passwd") << std::endl;
    
    return 0;
}
