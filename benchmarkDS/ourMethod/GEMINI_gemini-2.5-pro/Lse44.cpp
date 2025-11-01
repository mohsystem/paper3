#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

// POSIX headers for secure file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h> // For strerror
#include <errno.h>

const char* SAFE_DIR = "/safe/";
const off_t MAX_FILE_SIZE = 1024 * 1024; // 1 MiB

// RAII wrapper for file descriptors
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
    FdGuard(FdGuard&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
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
 * Reads a file from a secure directory, preventing path traversal and other attacks.
 *
 * @param filename The name of the file to read. Must not contain path separators.
 * @return The content of the file as a string.
 * @throws std::runtime_error on any failure.
 */
std::string readFileFromSafeDirectory(const std::string& filename) {
    // Rule #3: Validate input
    if (filename.empty() || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        throw std::runtime_error("Invalid filename: cannot be empty or contain path separators.");
    }
    
    // Get a file descriptor for the safe base directory
    FdGuard dir_fd(open(SAFE_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (dir_fd.get() < 0) {
        throw std::runtime_error(std::string("Failed to open safe directory: ") + strerror(errno));
    }

    // Rule #2: Open-then-validate pattern to avoid TOCTOU
    // O_NOFOLLOW prevents following symbolic links
    FdGuard file_fd(openat(dir_fd.get(), filename.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    if (file_fd.get() < 0) {
        throw std::runtime_error(std::string("Failed to open file '") + filename + "': " + strerror(errno));
    }

    // Validate the opened handle (file descriptor)
    struct stat st;
    if (fstat(file_fd.get(), &st) != 0) {
        throw std::runtime_error(std::string("Failed to stat file: ") + strerror(errno));
    }
    
    // Rule #1: Ensure it's a regular file
    if (!S_ISREG(st.st_mode)) {
        throw std::runtime_error("Not a regular file.");
    }

    // Rule #5: Check file size against a limit
    if (st.st_size > MAX_FILE_SIZE) {
        throw std::runtime_error("File size exceeds limit of " + std::to_string(MAX_FILE_SIZE) + " bytes.");
    }
    
    if (st.st_size == 0) {
        return "";
    }

    std::string content;
    content.resize(st.st_size);
    
    ssize_t total_bytes_read = 0;
    while(total_bytes_read < st.st_size) {
        ssize_t bytes_read = read(file_fd.get(), &content[total_bytes_read], st.st_size - total_bytes_read);
        if (bytes_read < 0) {
            if (errno == EINTR) continue; // Interrupted by signal, retry
            throw std::runtime_error(std::string("Failed to read file: ") + strerror(errno));
        }
        if (bytes_read == 0) { // EOF, should not happen if size is correct
             break;
        }
        total_bytes_read += bytes_read;
    }
    
    if(total_bytes_read != st.st_size) {
        content.resize(total_bytes_read);
    }
    
    return content;
}

int main() {
    /*
     * To run these test cases, you need to set up the environment.
     * You may need root permissions to create /safe.
     *
     * sudo mkdir -p /safe/subdir
     * sudo sh -c 'echo "This is a valid file." > /safe/valid_file.txt'
     * sudo sh -c 'echo "This is outside the safe dir." > /outside_file.txt'
     * sudo ln -s /safe/valid_file.txt /safe/symlink_to_valid
     * sudo chmod 600 /safe/valid_file.txt
     */
    std::vector<std::string> test_filenames = {
        "valid_file.txt",                // 1. Should succeed
        "non_existent_file.txt",         // 2. Should fail (file not found)
        "../outside_file.txt",           // 3. Should fail (invalid filename)
        "/etc/passwd",                   // 4. Should fail (invalid filename)
        "symlink_to_valid",              // 5. Should fail (is a symlink, O_NOFOLLOW)
        "subdir"                         // 6. Should fail (is a directory)
    };

    for (const auto& filename : test_filenames) {
        std::cout << "--- Testing: " << filename << " ---" << std::endl;
        try {
            std::string content = readFileFromSafeDirectory(filename);
            std::cout << "Success! Read " << content.length() << " bytes." << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "Failed as expected: " << e.what() << std::endl;
        }
    }

    return 0;
}