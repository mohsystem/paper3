#include <iostream>
#include <string>
#include <vector>
#include <cstdio> // For remove()

// POSIX headers for file control
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// For error reporting
#include <cerrno>
#include <cstring>

/**
 * @brief Opens/creates a file for reading and writing with specific permissions, 
 *        and writes content to it.
 * 
 * @param filename The path to the file.
 * @param content The string content to write.
 * @return true if the operation was successful, false otherwise.
 */
bool createAndWriteFile(const std::string& filename, const std::string& content) {
    // Flags: Open for Read/Write, Create if not exists, Truncate if exists.
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    
    // Permissions: rw-r--r-- (0644)
    // Corresponds to S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    // The third argument (mode) is only used when O_CREAT is specified.
    int fd = open(filename.c_str(), flags, mode);
    if (fd == -1) {
        std::cerr << "Error opening file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }

    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    
    // It's crucial to attempt closing the file descriptor even if write fails.
    int close_status = close(fd);

    if (bytes_written == -1) {
        std::cerr << "Error writing to file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }

    if (static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Error: Partial write to file '" << filename << "'" << std::endl;
        return false;
    }
    
    if (close_status == -1) {
        std::cerr << "Error closing file '" << filename << "': " << strerror(errno) << std::endl;
        return false; // The write might have succeeded, but close failed.
    }

    return true;
}

int main() {
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"important_config", "important_config"},
        {"test_cpp_1.cfg", "some_data_1"},
        {"test_cpp_2.cfg", "some_data_2"},
        {"test_cpp_3.cfg", "some_data_3"},
        {"test_cpp_4.cfg", "some_data_4"}
    };

    for (const auto& testCase : testCases) {
        const std::string& filename = testCase.first;
        const std::string& content = testCase.second;

        std::cout << "--- C++ Test Case: " << filename << " ---" << std::endl;
        bool success = createAndWriteFile(filename, content);
        std::cout << "Operation successful: " << (success ? "true" : "false") << std::endl;

        if (success) {
            if (remove(filename.c_str()) != 0) {
                std::cerr << "Failed to clean up file: " << filename << std::endl;
            } else {
                std::cout << "Cleaned up file: " << filename << std::endl;
            }
        }
        std::cout << std::endl;
    }

    return 0;
}