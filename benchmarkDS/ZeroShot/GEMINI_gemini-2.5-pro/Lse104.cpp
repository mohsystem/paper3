#include <iostream>
#include <string>
#include <vector>

// Required for POSIX file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

/**
 * Opens or creates a file for reading and writing, then writes content to it.
 *
 * @param filename The path to the file.
 * @param content The string content to write.
 * @return true on success, false on failure.
 */
bool secureWriteToFile(const std::string& filename, const std::string& content) {
    if (filename.empty()) {
        std::cerr << "Error: Filename cannot be empty." << std::endl;
        return false;
    }

    // O_RDWR: Open for reading and writing.
    // O_CREAT: Create the file if it does not exist.
    // S_IRUSR | S_IWUSR: Permissions for new file (0600) - owner can read/write.
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        std::cerr << "Error opening file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }

    const char* buffer = content.c_str();
    ssize_t bytes_to_write = content.length();
    ssize_t bytes_written = write(fd, buffer, bytes_to_write);

    if (bytes_written == -1) {
        std::cerr << "Error writing to file '" << filename << "': " << strerror(errno) << std::endl;
        close(fd); // Ensure cleanup on failure
        return false;
    }

    if (bytes_written != bytes_to_write) {
        std::cerr << "Error: Incomplete write to file '" << filename << "'." << std::endl;
        close(fd); // Ensure cleanup on failure
        return false;
    }

    if (close(fd) == -1) {
        std::cerr << "Error closing file '" << filename << "': " << strerror(errno) << std::endl;
        // The write was successful, but closing failed. Depending on policy,
        // this might still be considered a failure.
        return false;
    }

    return true;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Create a new file and write content.
    std::cout << "Test 1: " << (secureWriteToFile("config1.txt", "important_config") ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 2: Create another file with different content.
    std::cout << "Test 2: " << (secureWriteToFile("config2.txt", "user=admin;password=secret") ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 3: Overwrite the beginning of the first file.
    std::cout << "Test 3: " << (secureWriteToFile("config1.txt", "new_config") ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 4: Attempt to write to a file in a non-existent directory (should fail).
    std::cout << "Test 4: " << (secureWriteToFile("non_existent_dir/config.txt", "data") ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 5: Write an empty string to a file.
    std::cout << "Test 5: " << (secureWriteToFile("config5.txt", "") ? "SUCCESS" : "FAILURE") << std::endl;

    return 0;
}