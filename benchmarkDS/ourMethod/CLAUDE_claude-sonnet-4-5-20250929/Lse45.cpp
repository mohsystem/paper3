
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

const std::string SAFE_DIRECTORY = "/safe/directory";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

/**
 * Reads a file from the safe directory with proper security controls.
 * @param filename The requested filename (relative path)
 * @return Vector of bytes containing file contents
 * @throws std::runtime_error if file cannot be read or security checks fail
 */
std::vector<unsigned char> readFileFromSafeDirectory(const std::string& filename) {
    // Input validation: reject empty filename
    if (filename.empty()) {
        throw std::invalid_argument("Filename cannot be empty");
    }
    
    // Resolve the safe directory to absolute path
    char safe_dir_real[PATH_MAX];
    if (realpath(SAFE_DIRECTORY.c_str(), safe_dir_real) == nullptr) {
        throw std::runtime_error("Cannot resolve safe directory");
    }
    std::string safe_dir(safe_dir_real);
    
    // Construct the requested path
    std::string requested_path = safe_dir + "/" + filename;
    
    // Resolve to absolute path to check for path traversal
    char requested_real[PATH_MAX];
    if (realpath(requested_path.c_str(), requested_real) == nullptr) {
        throw std::runtime_error("File does not exist or cannot be resolved");
    }
    std::string resolved_path(requested_real);
    
    // Security check: ensure resolved path is within safe directory
    if (resolved_path.find(safe_dir) != 0 || 
        (resolved_path.length() > safe_dir.length() && 
         resolved_path[safe_dir.length()] != '/')) {
        throw std::runtime_error("Access denied: path traversal detected");
    }
    
    // Open file with O_RDONLY, O_CLOEXEC, and O_NOFOLLOW (race-safe)
    int fd = open(resolved_path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        throw std::runtime_error("Cannot open file");
    }
    
    // RAII-like cleanup using try-catch
    try {
        // Validate the opened file descriptor with fstat
        struct stat st;
        if (fstat(fd, &st) != 0) {
            close(fd);
            throw std::runtime_error("Cannot stat file");
        }
        
        // Check if it's a regular file
        if (!S_ISREG(st.st_mode)) {
            close(fd);
            throw std::runtime_error("Not a regular file");
        }
        
        // Check file size to prevent excessive memory allocation
        if (st.st_size > static_cast<off_t>(MAX_FILE_SIZE)) {
            close(fd);
            throw std::runtime_error("File size exceeds maximum allowed size");
        }
        
        // Allocate buffer for file contents
        std::vector<unsigned char> buffer(st.st_size);
        
        // Read file contents
        ssize_t bytes_read = 0;
        ssize_t total_read = 0;
        while (total_read < st.st_size) {
            bytes_read = read(fd, buffer.data() + total_read, st.st_size - total_read);
            if (bytes_read < 0) {
                close(fd);
                throw std::runtime_error("Error reading file");
            }
            if (bytes_read == 0) {
                break; // EOF
            }
            total_read += bytes_read;
        }
        
        close(fd);
        buffer.resize(total_read);
        return buffer;
        
    } catch (...) {
        close(fd);
        throw;
    }
}

int main() {
    // Test case 1: Valid file in safe directory
    try {
        std::vector<unsigned char> content = readFileFromSafeDirectory("test.txt");
        std::cout << "Test 1 - Valid file: Success (" << content.size() << " bytes)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 - Valid file: " << e.what() << std::endl;
    }
    
    // Test case 2: Attempt path traversal attack
    try {
        std::vector<unsigned char> content = readFileFromSafeDirectory("../../../etc/passwd");
        std::cout << "Test 2 - Path traversal: Failed (should be blocked)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 - Path traversal: Blocked successfully" << std::endl;
    }
    
    // Test case 3: Empty filename
    try {
        std::vector<unsigned char> content = readFileFromSafeDirectory("");
        std::cout << "Test 3 - Empty filename: Failed (should be rejected)" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 3 - Empty filename: Rejected successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 - Empty filename: " << e.what() << std::endl;
    }
    
    // Test case 4: Symlink attempt
    try {
        std::vector<unsigned char> content = readFileFromSafeDirectory("symlink.txt");
        std::cout << "Test 4 - Symlink: Failed (should be blocked)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Symlink: Blocked successfully" << std::endl;
    }
    
    // Test case 5: Subdirectory access (valid)
    try {
        std::vector<unsigned char> content = readFileFromSafeDirectory("subdir/file.txt");
        std::cout << "Test 5 - Subdirectory file: Success (" << content.size() << " bytes)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Subdirectory file: " << e.what() << std::endl;
    }
    
    return 0;
}
