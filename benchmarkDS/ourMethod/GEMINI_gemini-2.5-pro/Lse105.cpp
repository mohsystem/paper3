#include <iostream>
#include <string>
#include <vector>
#include <cstdio> // For std::remove
#include <system_error>

// POSIX headers for low-level file I/O
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

/**
 * Creates a file with the given name and content, setting restrictive permissions atomically.
 * This implementation is for POSIX-compliant systems.
 *
 * @param filename The name of the file to create.
 * @param content  The string content to write to the file.
 * @return true if the file was created successfully, false otherwise.
 */
bool createSecretFile(const std::string& filename, const std::string& content) {
    // Flags for atomic, exclusive creation with write-only access.
    // O_CREAT: Create file if it does not exist.
    // O_EXCL: Error if file exists.
    // O_WRONLY: Open for writing only.
    int flags = O_WRONLY | O_CREAT | O_EXCL;
    
    // Permissions: S_IRUSR | S_IWUSR corresponds to 0600 (read/write for owner only).
    mode_t mode = S_IRUSR | S_IWUSR;

    // open() is a POSIX system call that can create a file atomically with permissions.
    int fd = open(filename.c_str(), flags, mode);

    if (fd == -1) {
        std::cerr << "Error: Could not create file '" << filename << "': " << std::strerror(errno) << std::endl;
        return false;
    }

    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    
    if (bytes_written == -1) {
        std::cerr << "Error: Could not write to file '" << filename << "': " << std::strerror(errno) << std::endl;
        close(fd); 
        unlink(filename.c_str()); // Attempt to clean up the partially written file
        return false;
    }

    if (static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Error: Incomplete write to file '" << filename << "'" << std::endl;
        close(fd);
        unlink(filename.c_str());
        return false;
    }

    if (close(fd) == -1) {
        std::cerr << "Warning: Could not close file descriptor for '" << filename << "': " << std::strerror(errno) << std::endl;
    }

    std::cout << "Successfully created and secured file: " << filename << std::endl;
    return true;
}

int main() {
    const std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    std::cout << "--- Running Test Cases ---" << std::endl;

    std::vector<std::string> testFiles;
    for (int i = 1; i <= 5; ++i) {
        testFiles.push_back("secret" + std::to_string(i) + ".txt");
    }

    // Test Cases 1-5: Create files
    for (const auto& filename : testFiles) {
        std::cout << "\n--- Test: Create " << filename << " ---" << std::endl;
        createSecretFile(filename, content);
    }
    
    // Additional Test: Try creating an existing file to test failure
    std::cout << "\n--- Test: Attempt to re-create " << testFiles[1] << " (should fail) ---" << std::endl;
    createSecretFile(testFiles[1], content);

    std::cout << "\n--- Cleaning up created files ---" << std::endl;
    for (const auto& filename : testFiles) {
        remove(filename.c_str());
    }
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}