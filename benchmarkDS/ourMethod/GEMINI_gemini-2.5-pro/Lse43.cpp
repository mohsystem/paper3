#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <cstring>

// POSIX headers for file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// This code is POSIX-specific (Linux, macOS, etc.)
#ifdef _WIN32
#error "This code is designed for POSIX systems and will not compile on Windows."
#endif

const std::string SAFE_DIRECTORY = "/safe/";
constexpr size_t MAX_FILE_SIZE = 1024 * 1024; // 1 MB
constexpr size_t BUFFER_SIZE = 8192;

// RAII wrapper for file descriptors to ensure they are always closed.
class FdGuard {
public:
    explicit FdGuard(int fd = -1) : fd_(fd) {}
    ~FdGuard() {
        if (fd_ != -1) {
            close(fd_);
        }
    }
    FdGuard(const FdGuard&) = delete;
    FdGuard& operator=(const FdGuard&) = delete;
    FdGuard(FdGuard&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    FdGuard& operator=(FdGuard&& other) noexcept {
        if (this != &other) {
            if (fd_ != -1) close(fd_);
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    int get() const { return fd_; }
private:
    int fd_;
};

/**
 * Securely reads a file from a designated safe directory using the
 * openat-then-fstat pattern.
 * @param filename The name of the file to read. Must be a relative path without slashes.
 * @return A vector of bytes with the file content. Throws an exception on error.
 */
std::vector<char> readFileFromSafeDirectory(const std::string& filename) {
    // Rule #4, #7: Input validation.
    if (filename.empty() || filename.find('/') != std::string::npos || filename == "." || filename == "..") {
        throw std::invalid_argument("Error: Invalid filename.");
    }

    FdGuard dir_fd(open(SAFE_DIRECTORY.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (dir_fd.get() < 0) {
        throw std::system_error(errno, std::generic_category(), "Error: Could not open safe directory " + SAFE_DIRECTORY);
    }

    // Rule #3: Open the resource first, preventing symlink following.
    FdGuard fd(openat(dir_fd.get(), filename.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    if (fd.get() < 0) {
        throw std::system_error(errno, std::generic_category(), "Error opening file '" + filename + "'");
    }

    // Rule #3: Validate the already-opened handle.
    struct stat st;
    if (fstat(fd.get(), &st) < 0) {
        throw std::system_error(errno, std::generic_category(), "Error getting status for '" + filename + "'");
    }

    // Rule #4: Reject if not a regular file.
    if (!S_ISREG(st.st_mode)) {
        throw std::runtime_error("Error: '" + filename + "' is not a regular file.");
    }

    // Rule #1: Check initial size against policy.
    if (st.st_size > MAX_FILE_SIZE) {
        throw std::runtime_error("Error: '" + filename + "' is too large.");
    }
    
    std::vector<char> content;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    size_t total_bytes_read = 0;

    while ((bytes_read = read(fd.get(), buffer, sizeof(buffer))) > 0) {
        total_bytes_read += bytes_read;
        if (total_bytes_read > MAX_FILE_SIZE) {
            throw std::runtime_error("Error: File grew beyond size limit during read.");
        }
        content.insert(content.end(), buffer, buffer + bytes_read);
    }

    if (bytes_read < 0) {
        throw std::system_error(errno, std::generic_category(), "Error reading file '" + filename + "'");
    }
    
    return content;
}

void setupTestEnvironment() {
    // This setup may require appropriate permissions to create directories/files.
    mkdir(SAFE_DIRECTORY.c_str(), 0755);
    
    std::string good_file_path = SAFE_DIRECTORY + "goodfile.txt";
    int fd = open(good_file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        write(fd, "hello world", 11);
        close(fd);
    }
    
    std::string outside_file_path = "/tmp/outside.txt";
    fd = open(outside_file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
     if (fd != -1) {
        write(fd, "secret", 6);
        close(fd);
    }

    std::string symlink_path = SAFE_DIRECTORY + "symlink.txt";
    unlink(symlink_path.c_str());
    symlink(outside_file_path.c_str(), symlink_path.c_str());

    std::string subdir_path = SAFE_DIRECTORY + "subdir";
    mkdir(subdir_path.c_str(), 0755);
}

void runTestCase(int num, const std::string& description, const std::string& filename) {
    std::cout << "\n" << num << ". " << description << ": '" << filename << "'" << std::endl;
    try {
        std::vector<char> content = readFileFromSafeDirectory(filename);
        std::string str_content(content.begin(), content.end());
        std::cout << "Success! Content: " << str_content << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed as expected: " << e.what() << std::endl;
    }
}

int main() {
    setupTestEnvironment();
    std::cout << "--- Running Test Cases ---" << std::endl;

    runTestCase(1, "Reading 'goodfile.txt' (should succeed)", "goodfile.txt");
    runTestCase(2, "Reading 'nonexistent.txt' (should fail)", "nonexistent.txt");
    runTestCase(3, "Reading 'symlink.txt' (should fail)", "symlink.txt");
    runTestCase(4, "Reading '../goodfile.txt' (should fail)", "../goodfile.txt");
    runTestCase(5, "Reading 'subdir' (should fail)", "subdir");

    return 0;
}