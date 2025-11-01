#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cerrno>

// For POSIX-specific file operations
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h> // For mkstemp

/**
 * Creates a file, writes secret information, and sets permissions to read-only for the owner.
 * @param secretInfo The secret string to write.
 * @return The path of the created file.
 * @throws std::runtime_error on failure.
 */
std::string createSecretFile(const std::string& secretInfo) {
    // A template for the temporary file name. mkstemp will modify it.
    char filename_template[] = "/tmp/secretfile-cpp.XXXXXX";
    
    // Create a unique temporary file securely.
    // mkstemp creates the file with 0600 permissions, preventing race conditions.
    int fd = mkstemp(filename_template);
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file: " + std::string(strerror(errno)));
    }

    // Write the secret info to the file.
    ssize_t bytes_written = write(fd, secretInfo.c_str(), secretInfo.length());
    if (bytes_written == -1) {
        int write_errno = errno;
        close(fd);
        unlink(filename_template); // Clean up on failure
        throw std::runtime_error("Failed to write to temporary file: " + std::string(strerror(write_errno)));
    }
    if (static_cast<size_t>(bytes_written) != secretInfo.length()) {
        close(fd);
        unlink(filename_template);
        throw std::runtime_error("Incomplete write to temporary file.");
    }

    // Change permissions to read-only for the owner (0400).
    // Using fchmod is safer as it operates on the file descriptor, avoiding TOCTOU bugs.
    if (fchmod(fd, S_IRUSR) == -1) {
        int chmod_errno = errno;
        close(fd);
        unlink(filename_template); // Clean up on failure
        throw std::runtime_error("Failed to change file permissions: " + std::string(strerror(chmod_errno)));
    }
    
    // Close the file descriptor.
    if (close(fd) == -1) {
        // This is unusual. Warn but proceed as the file is usable.
        std::cerr << "Warning: Failed to close file descriptor: " << strerror(errno) << std::endl;
    }

    return std::string(filename_template);
}

int main() {
    std::cout << "Running C++ test cases..." << std::endl;
    std::vector<std::string> createdFiles;

    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::string secret = "C++ super secret info #" + std::to_string(i + 1);
        try {
            std::string fileName = createSecretFile(secret);
            std::cout << "Secret file created: " << fileName << std::endl;
            createdFiles.push_back(fileName);
            
            // Optional: Verify permissions using stat
            struct stat st;
            if (stat(fileName.c_str(), &st) == 0) {
                // Print permissions in octal
                std::cout << "File permissions (octal): " << std::oct << (st.st_mode & 0777) << std::dec << std::endl;
            } else {
                 std::cerr << "Could not stat file: " << strerror(errno) << std::endl;
            }

        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    // Cleanup created files
    std::cout << "\nCleaning up created files..." << std::endl;
    for (const auto& file : createdFiles) {
        if (unlink(file.c_str()) == 0) {
            std::cout << "Removed: " << file << std::endl;
        } else {
            std::cerr << "Error removing file " << file << ": " << strerror(errno) << std::endl;
        }
    }
    std::cout << "\nC++ tests finished." << std::endl;

    return 0;
}