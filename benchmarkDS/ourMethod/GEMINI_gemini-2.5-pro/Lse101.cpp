#include <iostream>
#include <string>
#include <vector>
#include <system_error>
#include <stdexcept>

// POSIX headers
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

// This code is intended for POSIX-compliant systems (e.g., Linux, macOS).

/**
 * Creates a file with the given name and writes the specified content to it securely.
 * This function uses a temporary file and an atomic rename operation to prevent
 * race conditions and ensure the file is written completely or not at all.
 *
 * @param fileName The name of the file to create.
 * @param content The content to write to the file.
 * @return true if the file was written successfully, false otherwise.
 */
bool writeImportantConfig(const std::string& fileName, const std::string& content) {
    std::string tempTemplateStr = fileName + ".XXXXXX";
    std::vector<char> tempTemplate(tempTemplateStr.begin(), tempTemplateStr.end());
    tempTemplate.push_back('\0');

    int fd = -1;
    int dir_fd = -1;
    bool success = false;

    // mkstemp creates a file with a unique name and opens it with 0600 permissions.
    fd = mkstemp(tempTemplate.data());
    if (fd == -1) {
        std::cerr << "Error: Could not create temporary file: " << strerror(errno) << std::endl;
        return false;
    }

    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    if (bytes_written < 0 || static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Error: Failed to write to temporary file: " << strerror(errno) << std::endl;
        goto cleanup_and_fail;
    }

    if (fsync(fd) == -1) {
        std::cerr << "Error: fsync failed for temporary file: " << strerror(errno) << std::endl;
        goto cleanup_and_fail;
    }

    if (close(fd) == -1) {
        std::cerr << "Error: Could not close temporary file: " << strerror(errno) << std::endl;
        fd = -1;
        goto cleanup_and_fail;
    }
    fd = -1; // Mark as closed, ownership transfers to rename/unlink

    if (rename(tempTemplate.data(), fileName.c_str()) == -1) {
        std::cerr << "Error: Could not rename temporary file: " << strerror(errno) << std::endl;
        goto cleanup_and_fail;
    }

    dir_fd = open(".", O_RDONLY | O_CLOEXEC);
    if (dir_fd == -1) {
        std::cerr << "Warning: Could not open parent directory to sync: " << strerror(errno) << std::endl;
    } else {
        if (fsync(dir_fd) == -1) {
            std::cerr << "Warning: Could not sync parent directory: " << strerror(errno) << std::endl;
        }
        close(dir_fd);
    }

    success = true;
    return success;

cleanup_and_fail:
    if (fd != -1) {
        close(fd);
    }
    // Remove the temporary file if it still exists because we failed before rename.
    unlink(tempTemplate.data());
    return false;
}

int main() {
    const std::string fileName = "important_config";
    const std::string content = "important_config";

    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test Case " << i << ":" << std::endl;
        bool success = writeImportantConfig(fileName, content);
        if (success) {
            std::cout << "Successfully wrote to " << fileName << std::endl;
        } else {
            std::cout << "Failed to write to " << fileName << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
    return 0;
}