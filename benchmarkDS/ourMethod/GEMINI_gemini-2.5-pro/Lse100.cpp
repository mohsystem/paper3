#include <iostream>
#include <string>
#include <vector>
#include <cstdio> // For std::remove and perror

// POSIX headers for file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>

/**
 * @brief Creates a file with secure permissions (0600: owner read/write only).
 *
 * This function uses POSIX open() with O_CREAT and O_EXCL flags to ensure
 * atomic file creation. It will fail if the file already exists, preventing
 * race conditions and overwriting existing files (including symlinks).
 * O_NOFOLLOW is used where available to prevent following symbolic links.
 *
 * @param filename The name of the file to create.
 * @return true if the file was created successfully, false otherwise.
 */
bool createSecureFile(const std::string& filename) {
    // O_CREAT: Create the file if it does not exist.
    // O_EXCL:  When used with O_CREAT, the call will fail if the file already exists.
    // O_WRONLY: Open for writing only.
    int flags = O_CREAT | O_EXCL | O_WRONLY;

// O_NOFOLLOW is a security enhancement but not in all POSIX standards.
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    // S_IRUSR | S_IWUSR corresponds to mode 0600 (read/write for owner only).
    mode_t mode = S_IRUSR | S_IWUSR;

    int fd = open(filename.c_str(), flags, mode);

    if (fd == -1) {
        perror(("Error creating file " + filename).c_str());
        return false;
    }

    // The file has been created successfully. We can now close the descriptor.
    close(fd);
    std::cout << "Successfully created file: " << filename << std::endl;
    return true;
}

int main() {
    std::vector<std::string> testFiles = {
        "important_config_1.txt",
        "important_config_2.cfg",
        "another_secure_file.dat",
        "test_config.ini",
        "user_settings.json"
    };
    
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    for (size_t i = 0; i < testFiles.size(); ++i) {
        const std::string& filename = testFiles[i];
        std::cout << "\n--- Test Case " << (i + 1) << ": " << filename << " ---" << std::endl;

        // 1. Attempt to create the file (should succeed)
        std::cout << "Attempt 1: Creating file." << std::endl;
        createSecureFile(filename);

        // 2. Attempt to create the same file again (should fail)
        std::cout << "Attempt 2: Creating the same file again." << std::endl;
        createSecureFile(filename);

        // 3. Clean up the created file
        if (std::remove(filename.c_str()) == 0) {
            std::cout << "Cleaned up file: " << filename << std::endl;
        } else {
            // Use cerr for errors
            std::cerr << "Failed to clean up file: " << filename << std::endl;
        }
    }
    
    return 0;
}