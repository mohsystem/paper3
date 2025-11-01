
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

std::string createSecureFile(const std::string& secretInfo, const std::string& filename) {
    try {
        // Create file with restrictive permissions (owner read-only: 0400)
        int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_EXCL, S_IRUSR);
        
        if (fd == -1) {
            return "Error: Could not create file";
        }
        
        // Write secret info
        write(fd, secretInfo.c_str(), secretInfo.length());
        close(fd);
        
        // Ensure permissions are set to read-only for owner
        chmod(filename.c_str(), S_IRUSR);
        
        return filename;
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    // Test cases
    std::string testCases[] = {
        "SuperSecret123!",
        "Confidential Data",
        "API_KEY=abc123xyz",
        "Password: P@ssw0rd!",
        "Token: eyJhbGciOiJIUzI1NiJ9"
    };
    
    for (int i = 0; i < 5; i++) {
        std::string filename = "secret_file_" + std::to_string(i + 1) + ".txt";
        std::string result = createSecureFile(testCases[i], filename);
        std::cout << result << std::endl;
    }
    
    return 0;
}
