#include <iostream>
#include <string>
#include <fstream>
#include <vector>

// POSIX headers for low-level file I/O
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Headers for error handling
#include <cerrno>
#include <cstring>

/**
 * Creates/opens a file with specific permissions (0644) and writes content.
 * The file is opened for reading and writing, created if it doesn't exist,
 * and truncated if it does.
 * 
 * @param filename The name of the file.
 * @param content The content to write to the file.
 * @return true on success, false on failure.
 */
bool createAndWriteConfig(const std::string& filename, const std::string& content) {
    // Flags: Read/Write, Create if not exists, Truncate if exists.
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    
    // Mode: Owner rw, Group r, Others r (0644 in octal)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    
    // The third argument (mode) is only used when O_CREAT is specified.
    int fd = open(filename.c_str(), flags, mode);
    
    if (fd == -1) {
        std::cerr << "Error opening file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }
    
    // Write content to the file descriptor
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    
    // Always close the file descriptor, even if write fails
    int close_status = close(fd);
    
    if (bytes_written == -1) {
        std::cerr << "Error writing to file '" << filename << "': " << strerror(errno) << std::endl;
        return false;
    }
    
    if (static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Error: Incomplete write to '" << filename << "'" << std::endl;
        return false;
    }

    if (close_status == -1) {
        std::cerr << "Error closing file '" << filename << "': " << strerror(errno) << std::endl;
        return false; // Though write succeeded, close failed, which is a problem
    }
    
    return true;
}

void runTestCase(int testNum) {
    const std::string filename = "important_config";
    const std::string content = "important_config";

    std::cout << "\n--- Test Case " << testNum << " ---" << std::endl;
    std::cout << "Attempting to write '" << content << "' to '" << filename << "'" << std::endl;
    
    if (createAndWriteConfig(filename, content)) {
        std::cout << "Successfully wrote to file." << std::endl;
        // Optional verification using C++ streams
        std::ifstream ifs(filename);
        if (ifs.is_open()) {
            std::string read_content;
            ifs >> read_content;
            std::cout << "File content verified: \"" << read_content << "\"" << std::endl;
        } else {
            std::cerr << "Failed to read back for verification." << std::endl;
        }
    } else {
        std::cerr << "Failed to write to file." << std::endl;
    }
}

int main() {
    std::cout << "Running 5 test cases..." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        runTestCase(i);
    }
    return 0;
}