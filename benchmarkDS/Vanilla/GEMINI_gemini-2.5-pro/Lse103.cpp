#include <iostream>
#include <fcntl.h>   // For O_CREAT, O_WRONLY, O_TRUNC, O_EXCL
#include <sys/stat.h> // For mode constants like S_IRUSR
#include <unistd.h>  // For open(), close(), write(), unlink()
#include <string.h>  // For strerror()
#include <errno.h>   // For errno

// Note: This code uses POSIX-specific functions (open, close, unlink).
// It will work on Linux, macOS, and other UNIX-like systems.
// For Windows, you may need to include <io.h> and use functions like _open(), _close().

/**
 * Opens a file with the specified POSIX flags and demonstrates the result.
 * @param pathname The path to the file.
 * @param flags A bitmask of the flags (e.g., O_CREAT | O_WRONLY).
 * @return true on success, false on failure.
 */
bool openFileWithFlags(const char* pathname, int flags) {
    // Permissions for new file: user read/write, group read, others read (0644)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    std::cout << "Attempting to open '" << pathname << "' with flags:";
    if (flags & O_CREAT) std::cout << " O_CREAT";
    if (flags & O_WRONLY) std::cout << " O_WRONLY";
    if (flags & O_TRUNC) std::cout << " O_TRUNC";
    if (flags & O_EXCL) std::cout << " O_EXCL";
    std::cout << std::endl;

    int fd = open(pathname, flags, mode);

    if (fd == -1) {
        std::cerr << "  -> Error: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "  -> Success: File opened/created with fd = " << fd << std::endl;
    const char* data = "hello";
    if (write(fd, data, strlen(data)) == -1) {
        std::cerr << "  -> Warning: Failed to write to file: " << strerror(errno) << std::endl;
    }
    close(fd);
    return true;
}

int main() {
    const char* filename = "cpp_test_file.txt";

    // Ensure the file does not exist before starting
    unlink(filename);

    std::cout << "--- C++ File Open Flags Demonstration (POSIX) ---" << std::endl;

    // Test 1: O_CREAT | O_WRONLY | O_EXCL on a new file
    std::cout << "\n[Test 1: Exclusive Create on new file - Should Succeed]" << std::endl;
    openFileWithFlags(filename, O_CREAT | O_WRONLY | O_EXCL);

    // Test 2: O_CREAT | O_WRONLY | O_EXCL on an existing file
    std::cout << "\n[Test 2: Exclusive Create on existing file - Should Fail]" << std::endl;
    openFileWithFlags(filename, O_CREAT | O_WRONLY | O_EXCL);

    // Test 3: O_CREAT | O_WRONLY on an existing file
    std::cout << "\n[Test 3: Open for Writing (O_WRONLY) - Should Succeed]" << std::endl;
    openFileWithFlags(filename, O_CREAT | O_WRONLY);

    // Test 4: O_CREAT | O_WRONLY | O_TRUNC on an existing file
    std::cout << "\n[Test 4: Open for Writing with Truncate (O_TRUNC) - Should Succeed]" << std::endl;
    openFileWithFlags(filename, O_CREAT | O_WRONLY | O_TRUNC);

    // Test 5: Cleanup
    std::cout << "\n[Test 5: Cleanup]" << std::endl;
    if (unlink(filename) == 0) {
        std::cout << "  -> Success: Deleted '" << filename << "'" << std::endl;
    } else {
        std::cerr << "  -> Error: Failed to delete '" << filename << "': " << strerror(errno) << std::endl;
    }

    return 0;
}