#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio> // For remove()

// For POSIX-specific file operations (O_EXCL) on Unix-like systems
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

/**
 * Creates or opens a file using POSIX open() to demonstrate specific flags.
 * O_CREAT: Create the file if it doesn't exist.
 * O_TRUNC: Truncate the file to 0 bytes if it already exists.
 * O_WRONLY: Open the file for writing only.
 * O_EXCL: When used with O_CREAT, fail if the file already exists.
 *
 * In a real-world application, sanitize the 'filePath' parameter to
 * prevent path traversal vulnerabilities.
 * 
 * @param filePath The path to the file.
 * @param flags A bitmask of flags for open().
 * @return true on success, false on failure.
 */
bool demonstratePosixOpen(const std::string& filePath, int flags) {
    // The mode is only used when O_CREAT is specified.
    // S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH corresponds to 0644 permissions.
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    
    // open() is a low-level system call.
    int fd = open(filePath.c_str(), flags, mode);

    if (fd == -1) {
        // strerror(errno) provides a human-readable error message.
        std::cerr << "Error: Failed to open '" << filePath << "': " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Success: Opened/created '" << filePath << "'. File descriptor: " << fd << std::endl;
    
    // Write some data to show the file is open for writing.
    const char* text = "CPP test content.\n";
    if (write(fd, text, strlen(text)) == -1) {
        std::cerr << "Error writing to file: " << strerror(errno) << std::endl;
    }
    
    // Always close the file descriptor.
    if (close(fd) == -1) {
        std::cerr << "Error closing file: " << strerror(errno) << std::endl;
    }
    
    return true;
}

void cleanupFiles(const std::vector<std::string>& files) {
    std::cout << "\n--- Cleanup ---" << std::endl;
    for (const auto& file : files) {
        remove(file.c_str());
    }
    std::cout << "Cleanup complete." << std::endl;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    std::string file1 = "test_cpp_1.txt";
    std::string file2 = "test_cpp_2.txt";

    // TC1: O_CREAT | O_WRONLY | O_TRUNC (Create new file, truncate if exists)
    std::cout << "\n[TC1] Creating new file with O_CREAT | O_WRONLY | O_TRUNC..." << std::endl;
    demonstratePosixOpen(file1, O_CREAT | O_WRONLY | O_TRUNC);
    
    // TC2: O_CREAT | O_WRONLY | O_TRUNC on an existing file (should truncate).
    std::cout << "\n[TC2] Opening existing file with O_TRUNC (should truncate)..." << std::endl;
    demonstratePosixOpen(file1, O_CREAT | O_WRONLY | O_TRUNC);

    // TC3: O_CREAT | O_WRONLY | O_EXCL (Create new file exclusively)
    std::cout << "\n[TC3] Creating new file with O_EXCL..." << std::endl;
    demonstratePosixOpen(file2, O_CREAT | O_WRONLY | O_EXCL);

    // TC4: O_CREAT | O_WRONLY | O_EXCL on an existing file (should fail).
    std::cout << "\n[TC4] Attempting to create existing file with O_EXCL (should fail)..." << std::endl;
    demonstratePosixOpen(file2, O_CREAT | O_WRONLY | O_EXCL);
    
    // TC5: O_WRONLY (Open existing file for writing without truncating)
    std::cout << "\n[TC5] Opening existing file with O_WRONLY (no truncate)..." << std::endl;
    demonstratePosixOpen(file1, O_WRONLY);

    cleanupFiles({file1, file2});
    return 0;
}