#include <iostream>
#include <string>
#include <vector>

// Platform-specific includes for file operations and system errors
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>
#endif

/**
 * Checks if a given path points to an existing directory.
 * @param path The directory path to check.
 * @return True if the directory exists, false otherwise.
 */
bool directoryExists(const std::string& path) {
    #ifdef _WIN32
        DWORD attribs = GetFileAttributesA(path.c_str());
        return (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
    #else
        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            return false;
        }
        return (info.st_mode & S_IFDIR);
    #endif
}

/**
 * Atomically creates a file with secure permissions (owner read/write only).
 * @param filePath The path to the file to be created.
 * @return True on success, false on failure.
 */
bool createSecureFile(const std::string& filePath) {
    if (filePath.empty()) {
        std::cerr << "Error: File path cannot be empty." << std::endl;
        return false;
    }
    
    // Security: Basic path traversal check
    if (filePath.find("..") != std::string::npos) {
        std::cerr << "Error: Invalid file path (contains '..')." << std::endl;
        return false;
    }

    // Check if parent directory exists
    size_t last_slash_idx = filePath.find_last_of("/\\");
    if (std::string::npos != last_slash_idx) {
        std::string dir = filePath.substr(0, last_slash_idx);
        if (!dir.empty() && !directoryExists(dir)) {
            std::cerr << "Error: Parent directory does not exist for path: " << filePath << std::endl;
            return false;
        }
    }

#ifdef _WIN32
    // Windows: Use secure _sopen_s with read/write permissions for the user.
    int pmode = _S_IREAD | _S_IWRITE;
    int fd;
    errno_t err = _sopen_s(&fd, filePath.c_str(), _O_CREAT | _O_EXCL | _O_WRONLY, _SH_DENYNO, pmode);
    if (err != 0) {
        if (err == EEXIST) {
            std::cerr << "Error: File already exists: " << filePath << std::endl;
        } else {
            char error_buffer[256];
            strerror_s(error_buffer, sizeof(error_buffer), err);
            std::cerr << "Error creating file '" << filePath << "': " << error_buffer << std::endl;
        }
        return false;
    }
    _close(fd);
#else
    // POSIX: Use open() with 0600 permissions.
    mode_t mode = S_IRUSR | S_IWUSR; // 0600 permissions
    int flags = O_WRONLY | O_CREAT | O_EXCL;
    int fd = open(filePath.c_str(), flags, mode);

    if (fd == -1) {
        std::cerr << "Error creating file '" << filePath << "': " << strerror(errno) << std::endl;
        return false;
    }
    close(fd);
#endif

    std::cout << "Successfully created secure file: " << filePath << std::endl;
    return true;
}

void runTestCase(int n, const std::string& path) {
    std::cout << "\nTest Case " << n << ": Create file '" << path << "'." << std::endl;
    createSecureFile(path);
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Standard successful creation
    runTestCase(1, "important_config.cpp");
    
    // Test Case 2: Attempt to create a file that already exists
    runTestCase(2, "important_config.cpp");

    // Test Case 3: Create a file with a relative path
    const char separator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif
    std::string relPath = "." + std::string(1, separator) + "another_config.cpp.cfg";
    runTestCase(3, relPath);

    // Test Case 4: Attempt to create a file in a non-existent directory
    std::string nonExistentPath = "non_existent_dir" + std::string(1, separator) + "some_file.txt";
    runTestCase(4, nonExistentPath);

    // Test Case 5: Another successful creation
    runTestCase(5, "config_3.cpp.dat");

    return 0;
}