#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio> // For remove()

// POSIX headers for file control
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Creates a file, writes a simple shell script, and makes it executable.
 * This implementation uses POSIX-specific functions (open, write, fchmod, close)
 * to securely create the file and set permissions, mitigating TOCTOU vulnerabilities.
 *
 * @param filename The name of the file to create. Must not contain path separators.
 * @return true on success, false on failure.
 */
bool createAndMakeExecutable(const std::string& filename) {
    if (filename.empty() || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        std::cerr << "Error: Invalid filename. It must not be empty or contain path separators." << std::endl;
        return false;
    }
    
    // Clean up file if it exists
    remove(filename.c_str());

    // Create the file with read/write permissions for the owner.
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        return false;
    }

    std::string content = "#!/bin/sh\necho \"Hello from " + filename + "!\"\n";
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    if (bytes_written == -1 || static_cast<size_t>(bytes_written) != content.length()) {
        perror("Error writing to file");
        close(fd);
        return false;
    }

    // Get current permissions
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("Error getting file status");
        close(fd);
        return false;
    }

    // Add execute permission for the owner
    if (fchmod(fd, file_stat.st_mode | S_IXUSR) == -1) {
        perror("Error setting file permissions");
        close(fd);
        return false;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return false;
    }

    return true;
}

int main() {
    std::vector<std::string> testFilenames = {
        "script1.sh",
        ".hidden_script.sh",
        "no_extension",
        "",                // Invalid: empty
        "test/script.sh"   // Invalid: contains path separator
    };

    for (const auto& filename : testFilenames) {
        std::cout << "Testing with filename: \"" << filename << "\"" << std::endl;
        bool success = createAndMakeExecutable(filename);
        std::cout << "Result: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
        if (success) {
            std::cout << "File '" << filename << "' created and made executable." << std::endl;
            // Clean up the created file
            if (remove(filename.c_str()) != 0) {
                perror("Error cleaning up file");
            } else {
                std::cout << "Cleaned up '" << filename << "'." << std::endl;
            }
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}