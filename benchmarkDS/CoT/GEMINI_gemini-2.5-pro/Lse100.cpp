#include <iostream>
#include <string>
#include <vector>
#include <cstdio> // For remove()
#include <cstring> // For strerror()
#include <cerrno>  // For errno

// POSIX headers for file control
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Atomically creates a file with secure permissions.
 *
 * This function uses low-level POSIX calls to ensure that the file is created
 * with the correct permissions (0600, owner read/write) in a single, atomic
 * operation. This prevents race conditions where a file could be accessed
 * between creation and permission setting.
 *
 * @param filePath The path of the file to create.
 * @return true if the file was created successfully, false otherwise.
 */
bool createSecureFile(const std::string& filePath) {
    // Mode for owner read/write (rw-------), equivalent to octal 0600.
    // S_IRUSR: Read permission for owner.
    // S_IWUSR: Write permission for owner.
    mode_t mode = S_IRUSR | S_IWUSR;
    
    // Flags for atomic creation:
    // O_WRONLY: Open for writing only.
    // O_CREAT: Create the file if it does not exist.
    // O_EXCL: When used with O_CREAT, the call will fail if the file already exists.
    //         This is crucial for preventing race conditions and overwrites.
    int flags = O_WRONLY | O_CREAT | O_EXCL;

    // The open() system call is atomic.
    int fd = open(filePath.c_str(), flags, mode);

    if (fd == -1) {
        // An error occurred. The global 'errno' variable is set.
        std::cerr << "Error creating file '" << filePath << "': " << strerror(errno) << std::endl;
        return false;
    }

    // The file was created successfully. We can close the file descriptor
    // immediately as we only needed to create the file, not write to it.
    close(fd);
    std::cout << "Successfully created secure file: " << filePath << std::endl;
    return true;
}

int main() {
    const std::string fileName1 = "important_config";
    const std::string fileName2 = "another_config";
    const std::string invalidPath = "non_existent_dir/important_config";
    const std::string relativePath = "./temp_config";

    std::cout << "--- Test Case 1: Create new file ---" << std::endl;
    createSecureFile(fileName1);

    std::cout << "\n--- Test Case 2: Attempt to create existing file ---" << std::endl;
    createSecureFile(fileName1);

    std::cout << "\n--- Test Case 3: Create a different new file ---" << std::endl;
    createSecureFile(fileName2);

    std::cout << "\n--- Test Case 4: Attempt to create file in non-existent directory ---" << std::endl;
    createSecureFile(invalidPath);

    std::cout << "\n--- Test Case 5: Create a file with a relative path ---" << std::endl;
    createSecureFile(relativePath);

    // Cleanup
    std::cout << "\n--- Cleaning up created files ---" << std::endl;
    remove(fileName1.c_str());
    remove(fileName2.c_str());
    remove(relativePath.c_str());
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}