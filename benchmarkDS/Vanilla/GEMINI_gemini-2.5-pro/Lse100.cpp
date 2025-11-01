#include <iostream>
#include <string>
#include <fcntl.h>   // For open() and O_flags
#include <sys/stat.h>  // For S_I permissions
#include <unistd.h>  // For close()
#include <cerrno>    // For errno
#include <cstring>   // For strerror
#include <cstdio>    // For remove()

// Note: This code uses POSIX-specific functions (open, S_IRUSR, etc.).
// It will compile and run on Linux, macOS, and other Unix-like systems.
// For a cross-platform solution on Windows, different APIs would be needed.

/**
 * @brief Creates a file with owner-only read/write permissions.
 * This function is idempotent: it returns true if the file is successfully
 * created or if it already exists.
 *
 * @param filename The name of the file to create.
 * @return true on success (created or exists), false on error.
 */
bool createConfigFile(const std::string& filename) {
    // Define permissions: read and write for the owner (0600)
    mode_t permissions = S_IRUSR | S_IWUSR;

    // Attempt to create the file exclusively.
    // O_CREAT: create if it doesn't exist
    // O_WRONLY: open for writing
    // O_EXCL: fail if the file already exists (atomic check-and-create)
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_EXCL, permissions);

    if (fd != -1) {
        // Success, the file was created.
        close(fd);
        std::cout << "Successfully created '" << filename << "' with owner read/write permissions." << std::endl;
        return true;
    }

    // If open failed, check the reason.
    if (errno == EEXIST) {
        // The file already exists, which we treat as a success.
        std::cout << "File '" << filename << "' already exists." << std::endl;
        return true;
    }

    // Another error occurred.
    std::cerr << "Error creating file '" << filename << "': " << strerror(errno) << std::endl;
    return false;
}

int main() {
    std::string fileName = "important_config";
    std::string otherFileName = "another_config.txt";

    std::cout << "--- Test Case 1: Create file for the first time ---" << std::endl;
    createConfigFile(fileName);

    std::cout << "\n--- Test Case 2: Try to create the same file again ---" << std::endl;
    createConfigFile(fileName);

    std::cout << "\n--- Test Case 3: Delete the file ---" << std::endl;
    if (remove(fileName.c_str()) == 0) {
        std::cout << "Deleted '" << fileName << "' for re-testing." << std::endl;
    }

    std::cout << "\n--- Test Case 4: Create the file again after deletion ---" << std::endl;
    createConfigFile(fileName);

    std::cout << "\n--- Test Case 5: Create a different file ---" << std::endl;
    createConfigFile(otherFileName);

    // Cleanup
    std::cout << "\n--- Cleanup ---" << std::endl;
    remove(fileName.c_str());
    remove(otherFileName.c_str());
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}