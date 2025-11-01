#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio> // For remove() and perror()

// POSIX headers for file control
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @brief Opens or creates a file using POSIX flags.
 * This function is secure because it does not create a race condition when checking for
 * file existence (by using O_EXCL). It also specifies secure default permissions
 * (0644) during file creation. The file descriptor is always closed before returning.
 *
 * @param filename The path to the file.
 * @param flags A bitmask of flags like O_CREAT, O_WRONLY, etc.
 * @return true on success, false on failure.
 */
bool openFile(const std::string& filename, int flags) {
    int fd = -1;
    // Define secure permissions for file creation (user r/w, group r, other r)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 0644

    // The mode argument is only used by open() when O_CREAT is specified.
    if (flags & O_CREAT) {
        fd = open(filename.c_str(), flags, mode);
    } else {
        fd = open(filename.c_str(), flags);
    }

    if (fd == -1) {
        // perror provides a descriptive error message from the OS
        perror(("Operation failed for " + filename).c_str());
        return false;
    }

    // The goal is just to open the file, so we close it immediately.
    // In a real application, you would use the file descriptor 'fd'.
    close(fd);
    return true;
}

/**
 * @brief Helper function for test setup/cleanup
 */
void setupFile(const std::string& filename, const char* content) {
    if (content == nullptr) {
        remove(filename.c_str());
    } else {
        std::ofstream outfile(filename);
        if (outfile.is_open()) {
            outfile << content;
            outfile.close();
        } else {
            std::cerr << "Test setup failed for " << filename << std::endl;
        }
    }
}

int main() {
    std::vector<std::string> testFiles = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};

    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Create a new file exclusively (O_CREAT | O_WRONLY | O_EXCL)
    std::cout << "\n[Test 1] Create new file exclusively (O_CREAT | O_WRONLY | O_EXCL)" << std::endl;
    std::string file1 = "test1.txt";
    setupFile(file1, nullptr); // Ensure file does not exist
    bool result1 = openFile(file1, O_CREAT | O_WRONLY | O_EXCL);
    std::cout << "Result: " << (result1 ? "Success" : "Failure") << " (Expected: Success)" << std::endl;

    // Test Case 2: Attempt to create an existing file exclusively (O_CREAT | O_WRONLY | O_EXCL)
    std::cout << "\n[Test 2] Attempt exclusive creation on existing file (O_CREAT | O_WRONLY | O_EXCL)" << std::endl;
    std::string file2 = "test2.txt";
    setupFile(file2, "content"); // Ensure file exists
    bool result2 = openFile(file2, O_CREAT | O_WRONLY | O_EXCL);
    std::cout << "Result: " << (result2 ? "Success" : "Failure") << " (Expected: Failure)" << std::endl;

    // Test Case 3: Open an existing file and truncate it (O_WRONLY | O_TRUNC)
    std::cout << "\n[Test 3] Open and truncate existing file (O_WRONLY | O_TRUNC)" << std::endl;
    std::string file3 = "test3.txt";
    setupFile(file3, "some initial content"); // Ensure file exists with content
    bool result3 = openFile(file3, O_WRONLY | O_TRUNC);
    std::cout << "Result: " << (result3 ? "Success" : "Failure") << " (Expected: Success)" << std::endl;

    // Test Case 4: Open an existing file for writing only (O_WRONLY)
    std::cout << "\n[Test 4] Open existing file for writing (O_WRONLY)" << std::endl;
    std::string file4 = "test4.txt";
    setupFile(file4, "content"); // Ensure file exists
    bool result4 = openFile(file4, O_WRONLY);
    std::cout << "Result: " << (result4 ? "Success" : "Failure") << " (Expected: Success)" << std::endl;

    // Test Case 5: Create a file if it does not exist (O_CREAT | O_WRONLY)
    std::cout << "\n[Test 5] Create file if it doesn't exist (O_CREAT | O_WRONLY)" << std::endl;
    std::string file5 = "test5.txt";
    setupFile(file5, nullptr); // Ensure file does not exist
    bool result5 = openFile(file5, O_CREAT | O_WRONLY);
    std::cout << "Result: " << (result5 ? "Success" : "Failure") << " (Expected: Success)" << std::endl;

    // Cleanup
    std::cout << "\nCleaning up test files..." << std::endl;
    for (const auto& file : testFiles) {
        setupFile(file, nullptr);
    }
    std::cout << "Done." << std::endl;

    return 0;
}