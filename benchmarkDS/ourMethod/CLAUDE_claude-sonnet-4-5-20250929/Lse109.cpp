
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// Security: Read /etc/passwd directly instead of executing shell commands
// This prevents command injection (CWE-78) by avoiding system() calls
std::string getUserInfo(const std::string& username) {
    if (username.empty()) {
        return "Error: Username cannot be empty";
    }
    
    // Security: Validate username to contain only safe characters
    // Usernames should only contain alphanumeric, underscore, hyphen, and period
    std::regex usernamePattern("^[a-zA-Z0-9._-]+$");
    if (!std::regex_match(username, usernamePattern)) {
        return "Error: Invalid username format";
    }
    
    // Security: Limit username length to prevent resource exhaustion
    if (username.length() > 32) {
        return "Error: Username too long";
    }
    
    const char* passwdPath = "/etc/passwd";
    
    // Security: Open file first, then validate (prevents TOCTOU)
    // Use O_RDONLY | O_NOFOLLOW to prevent symlink following
    int fd = open(passwdPath, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        return "Error: Cannot open /etc/passwd";
    }
    
    // Security: Validate that the opened file is a regular file
    struct stat fileStat;
    if (fstat(fd, &fileStat) != 0 || !S_ISREG(fileStat.st_mode)) {
        close(fd);
        return "Error: /etc/passwd is not a regular file";
    }
    
    // Security: Check file size to prevent excessive memory allocation
    if (fileStat.st_size > 10 * 1024 * 1024) { // 10MB limit
        close(fd);
        return "Error: File too large";
    }
    
    // Security: Use fdopen to work with the already-opened file descriptor
    FILE* file = fdopen(fd, "r");
    if (!file) {
        close(fd);
        return "Error: Cannot read /etc/passwd";
    }
    
    std::string result;
    char line[1024]; // Security: Fixed size buffer with bounds checking
    bool found = false;
    int lineCount = 0;
    const int maxLines = 10000; // Security: Limit lines read
    
    // Security: Use fgets for bounds-checked reading
    while (fgets(line, sizeof(line), file) && lineCount < maxLines) {
        lineCount++;
        
        // Security: Ensure null termination
        line[sizeof(line) - 1] = '\\0';
        
        std::string lineStr(line);
        
        // Remove newline
        if (!lineStr.empty() && lineStr[lineStr.length() - 1] == '\\n') {
            lineStr.erase(lineStr.length() - 1);
        }
        
        // Parse line format: username:password:uid:gid:gecos:home:shell
        size_t colonPos = lineStr.find(':');
        if (colonPos != std::string::npos) {
            std::string fileUsername = lineStr.substr(0, colonPos);
            if (fileUsername == username) {
                result += lineStr + "\\n";
                found = true;
            }
        }
    }
    
    fclose(file); // Also closes fd
    
    if (!found) {
        return "User not found: " + username;
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testUsers[] = {"root", "daemon", "bin", "nonexistent", "test.user"};
    
    std::cout << "=== User Information Lookup ===" << std::endl << std::endl;
    
    for (const char* user : testUsers) {
        std::cout << "Looking up user: " << user << std::endl;
        std::string info = getUserInfo(user);
        std::cout << info << std::endl;
        std::cout << "---" << std::endl;
    }
    
    // If command line argument provided, use it
    if (argc > 1) {
        std::cout << "\\nCommand line lookup: " << argv[1] << std::endl;
        std::cout << getUserInfo(argv[1]) << std::endl;
    }
    
    return 0;
}
