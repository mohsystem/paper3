#include <iostream>
#include <string>
#include <vector>
#include <cstring> // For strerror
#include <cerrno>  // For errno

// POSIX headers for file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @brief Creates a file if it doesn't exist, opens it for reading/writing,
 * and writes content to it.
 *
 * This function follows secure coding practices:
 * - It uses open() with restrictive permissions (0600).
 * - It validates the file descriptor immediately after opening using fstat()
 *   to ensure it's a regular file, preventing TOCTOU vulnerabilities.
 * - It checks the return values of all system calls.
 *
 * @param path The path to the file.
 * @param content The string content to write.
 * @return true on success, false on failure.
 */
bool createAndWriteFile(const std::string& path, const std::string& content) {
    // Open the file with flags: Read/Write, Create if not exists.
    // Set restrictive permissions (S_IRUSR | S_IWUSR corresponds to 0600).
    // O_NOFOLLOW prevents opening a symbolic link, which is a security best practice.
    int fd = open(path.c_str(), O_RDWR | O_CREAT | O_NOFOLLOW, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        std::cerr << "Error opening file " << path << ": " << strerror(errno) << std::endl;
        return false;
    }

    // After opening, immediately validate the handle (file descriptor).
    struct stat st;
    if (fstat(fd, &st) == -1) {
        std::cerr << "Error getting file stats for " << path << ": " << strerror(errno) << std::endl;
        close(fd);
        // Attempt to remove the file that was just created but is in an error state.
        remove(path.c_str());
        return false;
    }

    // Ensure we are operating on a regular file.
    if (!S_ISREG(st.st_mode)) {
        std::cerr << "Error: " << path << " is not a regular file." << std::endl;
        close(fd);
        // Clean up the created file system entry.
        remove(path.c_str());
        return false;
    }

    // Write the content to the file.
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    if (bytes_written == -1) {
        std::cerr << "Error writing to file " << path << ": " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    if (static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Error: Incomplete write to file " << path << std::endl;
        close(fd);
        return false;
    }

    // Close the file descriptor.
    if (close(fd) == -1) {
        std::cerr << "Error closing file " << path << ": " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

int main() {
    const std::string important_config = "important_config=true";

    // Test cases
    const std::vector<std::string> test_files = {
        "test_config1.txt",
        "test_config2.txt",
        "test_config3.txt",
        "test_config4.txt",
        "test_config5.txt"
    };

    for (const auto& filename : test_files) {
        std::cout << "Attempting to write to: " << filename << std::endl;
        if (createAndWriteFile(filename, important_config)) {
            std::cout << "Successfully wrote to " << filename << std::endl;
            // Clean up the test file
            if (remove(filename.c_str()) != 0) {
                 std::cerr << "Error cleaning up file: " << filename << std::endl;
            } else {
                 std::cout << "Cleaned up " << filename << std::endl;
            }
        } else {
            std::cerr << "Failed to write to " << filename << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}