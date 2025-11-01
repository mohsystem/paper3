#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cerrno>

// POSIX headers for file operations
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

// Define a maximum file size to prevent large file attacks
const long MAX_FILE_SIZE = 1024 * 1024; // 1 MB

// RAII wrapper for file descriptors
class FdGuard {
public:
    explicit FdGuard(int fd) : fd_(fd) {}
    ~FdGuard() {
        if (fd_ >= 0) {
            close(fd_);
        }
    }
    // Disable copying
    FdGuard(const FdGuard&) = delete;
    FdGuard& operator=(const FdGuard&) = delete;

private:
    int fd_;
};

/**
 * Reads a file from a safe directory using file descriptors to prevent TOCTOU
 * and path traversal vulnerabilities.
 *
 * @param safeDir The path to the trusted directory.
 * @param filename The name of the file to read.
 * @return A vector of chars containing the file content.
 * @throws std::runtime_error on failure.
 */
std::vector<char> readFileFromSafeDirectory(const std::string& safeDir, const std::string& filename) {
    // 1. Validate filename to ensure it is a simple name, not a path.
    if (filename.find('/') != std::string::npos || filename.find("..") != std::string::npos) {
        throw std::runtime_error("Error: Filename must not contain path components.");
    }

    // 2. Get a file descriptor for the safe directory.
    int dir_fd = open(safeDir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd < 0) {
        throw std::runtime_error("Error: Could not open safe directory: " + std::string(strerror(errno)));
    }
    FdGuard dirFdGuard(dir_fd);

    // 3. Open the file relative to the directory descriptor.
    //    O_NOFOLLOW prevents following symbolic links.
    int fd = openat(dir_fd, filename.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        throw std::runtime_error("Error: Could not open file '" + filename + "': " + std::string(strerror(errno)));
    }
    FdGuard fileFdGuard(fd);

    // 4. Validate the opened file handle using fstat.
    struct stat st;
    if (fstat(fd, &st) != 0) {
        throw std::runtime_error("Error: fstat failed for '" + filename + "': " + std::string(strerror(errno)));
    }

    if (!S_ISREG(st.st_mode)) {
        throw std::runtime_error("Error: '" + filename + "' is not a regular file.");
    }

    if (st.st_size > MAX_FILE_SIZE) {
        throw std::runtime_error("Error: File '" + filename + "' exceeds size limit.");
    }
    if (st.st_size < 0) {
        throw std::runtime_error("Error: Invalid file size for '" + filename + "'.");
    }

    // 5. Read from the validated file descriptor.
    std::vector<char> buffer(st.st_size);
    if (st.st_size > 0) {
        ssize_t bytes_read = read(fd, buffer.data(), st.st_size);
        if (bytes_read < 0) {
            throw std::runtime_error("Error: read failed for '" + filename + "': " + std::string(strerror(errno)));
        }
        if (bytes_read != st.st_size) {
            throw std::runtime_error("Error: Incomplete read for '" + filename + "'.");
        }
    }
    
    return buffer;
}

void setup_test_environment(const std::string& dir, const std::string& secret) {
    mkdir(dir.c_str(), 0755);
    int fd = creat((dir + "/good.txt").c_str(), 0644);
    if(fd != -1) {
        write(fd, "This is a safe file.", 20);
        close(fd);
    }
    fd = creat(secret.c_str(), 0644);
    if(fd != -1) {
        write(fd, "This is a secret.", 17);
        close(fd);
    }
    std::string large_file_path = dir + "/large.txt";
    fd = creat(large_file_path.c_str(), 0644);
    if(fd != -1) {
        std::vector<char> large_content(MAX_FILE_SIZE + 1, 0);
        write(fd, large_content.data(), large_content.size());
        close(fd);
    }
    symlink(secret.c_str(), (dir + "/symlink.txt").c_str());
}

void cleanup_test_environment(const std::string& dir, const std::string& secret) {
    unlink((dir + "/good.txt").c_str());
    unlink((dir + "/large.txt").c_str());
    unlink((dir + "/symlink.txt").c_str());
    unlink(secret.c_str());
    rmdir(dir.c_str());
}

int main() {
    const std::string safeDirName = "cpp_safe_dir";
    const std::string secretFileName = "cpp_secret_file.txt";

    setup_test_environment(safeDirName, secretFileName);

    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Read a valid file
    std::cout << "\n1. Reading a valid file (good.txt):" << std::endl;
    try {
        std::vector<char> content = readFileFromSafeDirectory(safeDirName, "good.txt");
        std::cout << "   Success. Content: " << std::string(content.begin(), content.end()) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "   Failed: " << e.what() << std::endl;
    }

    // Test Case 2: Read a non-existent file
    std::cout << "\n2. Reading a non-existent file (nonexistent.txt):" << std::endl;
    try {
        readFileFromSafeDirectory(safeDirName, "nonexistent.txt");
        std::cerr << "   Failed: Code read a non-existent file." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   Success: Read operation failed as expected. (" << e.what() << ")" << std::endl;
    }

    // Test Case 3: Read a file that is too large
    std::cout << "\n3. Reading a large file (large.txt):" << std::endl;
    try {
        readFileFromSafeDirectory(safeDirName, "large.txt");
        std::cerr << "   Failed: Code read an oversized file." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   Success: Read operation failed as expected. (" << e.what() << ")" << std::endl;
    }

    // Test Case 4: Attempt path traversal
    std::cout << "\n4. Attempting path traversal (../" << secretFileName << "):" << std::endl;
    try {
        readFileFromSafeDirectory(safeDirName, "../" + secretFileName);
        std::cerr << "   Failed: Path traversal was successful." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   Success: Path traversal was blocked. (" << e.what() << ")" << std::endl;
    }

    // Test Case 5: Attempt to read a symbolic link
    std::cout << "\n5. Reading a symbolic link (symlink.txt):" << std::endl;
    try {
        readFileFromSafeDirectory(safeDirName, "symlink.txt");
        std::cerr << "   Failed: Code followed a symbolic link." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   Success: Reading symbolic link was blocked. (" << e.what() << ")" << std::endl;
    }
    
    cleanup_test_environment(safeDirName, secretFileName);
    return 0;
}