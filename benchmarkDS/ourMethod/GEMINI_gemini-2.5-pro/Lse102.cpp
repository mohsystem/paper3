#include <iostream>
#include <string>
#include <vector>
#include <system_error>

// POSIX headers for low-level file I/O
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

/**
 * Creates a file if it doesn't exist with specific permissions (0644),
 * opens it for reading/writing, and writes content to it.
 * This function is designed to be secure against TOCTOU vulnerabilities
 * by using O_NOFOLLOW and fstat.
 *
 * @param filename The name of the file to open/create.
 * @param content  The string content to write to the file.
 * @return true on success, false on failure.
 */
bool createAndWriteFile(const std::string& filename, const std::string& content) {
    // Define flags: Read/Write, Create if not exists, and do not follow symlinks.
    // O_NOFOLLOW is a crucial security feature to prevent symlink attacks.
    int flags = O_RDWR | O_CREAT | O_NOFOLLOW;

    // Define permissions: S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH (0644)
    // Owner: read/write, Group: read, Others: read
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    // Open the file, getting a file descriptor.
    int fd = open(filename.c_str(), flags, mode);
    if (fd == -1) {
        std::cerr << "Error opening file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }

    // Security check: After opening, validate the file handle.
    // This prevents race conditions where the file could be replaced
    // by a symlink or directory between a check and its use.
    struct stat st;
    if (fstat(fd, &st) == -1) {
        std::cerr << "Error getting file stats for '" << filename << "': " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    if (!S_ISREG(st.st_mode)) {
        std::cerr << "Error: '" << filename << "' is not a regular file." << std::endl;
        close(fd);
        return false;
    }

    // Truncate the file to 0 bytes before writing to ensure it only contains new content.
    if (ftruncate(fd, 0) == -1) {
        std::cerr << "Error truncating file '" << filename << "': " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    // Write the content to the file.
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    if (bytes_written == -1) {
        std::cerr << "Error writing to file '" << filename << "': " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    if (static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Error: Partial write to '" << filename << "'." << std::endl;
        close(fd);
        return false;
    }

    // Close the file descriptor.
    if (close(fd) == -1) {
        std::cerr << "Error closing file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

int main() {
    // Test Case 1: The primary specified case
    std::cout << "Test Case 1: important_config" << std::endl;
    if (createAndWriteFile("important_config", "important_config")) {
        std::cout << "  Success." << std::endl;
    } else {
        std::cout << "  Failure." << std::endl;
    }

    // Test Case 2: A different filename and content
    std::cout << "\nTest Case 2: another_config.txt" << std::endl;
    if (createAndWriteFile("another_config.txt", "some other data")) {
        std::cout << "  Success." << std::endl;
    } else {
        std::cout << "  Failure." << std::endl;
    }

    // Test Case 3: Empty content
    std::cout << "\nTest Case 3: empty_file.cfg" << std::endl;
    if (createAndWriteFile("empty_file.cfg", "")) {
        std::cout << "  Success." << std::endl;
    } else {
        std::cout << "  Failure." << std::endl;
    }

    // Test Case 4: File with special characters in name
    std::cout << "\nTest Case 4: file-with-hyphen.log" << std::endl;
    if (createAndWriteFile("file-with-hyphen.log", "log entry 1")) {
        std::cout << "  Success." << std::endl;
    } else {
        std::cout << "  Failure." << std::endl;
    }
        
    // Test Case 5: Overwriting an existing file
    std::cout << "\nTest Case 5: Overwriting important_config" << std::endl;
    if (createAndWriteFile("important_config", "new content")) {
        std::cout << "  Success." << std::endl;
    } else {
        std::cout << "  Failure." << std::endl;
    }

    return 0;
}