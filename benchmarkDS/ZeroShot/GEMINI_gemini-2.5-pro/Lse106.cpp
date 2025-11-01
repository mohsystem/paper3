#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

// The secure file creation functions used here (mkstemp, chmod) are POSIX-specific.
// This code will not compile on a standard Windows environment without a POSIX layer like Cygwin or WSL.
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/stat.h>

/**
 * @brief Creates a file, writes secret information to it, and changes its permissions
 * to be read-only for the owner.
 *
 * @param secretInfo The string content to write to the file.
 * @return The path of the created file.
 * @throws std::runtime_error on failure.
 */
std::string createSecretFile(const std::string& secretInfo) {
    // 1. Create a template for a secure temporary file.
    // mkstemp requires a mutable C-style string.
    std::string temp_template_str = "/tmp/secret_XXXXXX";
    std::vector<char> temp_path_c(temp_template_str.begin(), temp_template_str.end());
    temp_path_c.push_back('\0');

    // mkstemp securely creates a file and returns a file descriptor.
    int fd = mkstemp(temp_path_c.data());
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file: " + std::string(strerror(errno)));
    }
    
    std::string filePath(temp_path_c.data());

    // 2. Write the secret info to the file using the file descriptor.
    ssize_t bytes_written = write(fd, secretInfo.c_str(), secretInfo.length());
    if (bytes_written != static_cast<ssize_t>(secretInfo.length())) {
        close(fd);
        unlink(filePath.c_str()); // Clean up file on failure
        throw std::runtime_error("Failed to write to temporary file.");
    }

    close(fd);

    // 3. Change permissions to read-only for the owner (0400).
    // S_IRUSR is the constant for owner read permission.
    if (chmod(filePath.c_str(), S_IRUSR) == -1) {
        unlink(filePath.c_str()); // Clean up file on failure
        throw std::runtime_error("Failed to change file permissions: " + std::string(strerror(errno)));
    }
    
    return filePath;
}

int main() {
    std::cout << "Running C++ Test Cases:" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        try {
            std::string secret = "C++ super secret key #" + std::to_string(i) + " is CPK" + std::to_string(i * 789);
            std::string fileName = createSecretFile(secret);
            std::cout << "Test Case " << i << ": Secret file created at: " << fileName << std::endl;
            // Note: In a real app, you would schedule this file for deletion when no longer needed.
        } catch (const std::runtime_error& e) {
            std::cerr << "Test Case " << i << ": Error: " << e.what() << std::endl;
        }
    }
    return 0;
}

#else
// Provide a main function for non-POSIX systems like Windows to allow compilation.
int main() {
    std::cout << "Running C++ Test Cases:" << std::endl;
    std::cerr << "This secure file creation example is designed for POSIX-compliant systems (Linux, macOS) and will not run on this platform." << std::endl;
    return 1;
}
#endif