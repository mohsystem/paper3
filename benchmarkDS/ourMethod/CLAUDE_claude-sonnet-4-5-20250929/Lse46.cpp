
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <cerrno>

// Base directory for images - must be validated
const std::string BASE_DIR = "images";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

// URL decode helper function
std::string url_decode(const std::string& encoded) {
    std::string decoded;
    size_t len = encoded.length();
    for (size_t i = 0; i < len; ++i) {
        if (encoded[i] == '%' && i + 2 < len) {
            int value;
            if (sscanf(encoded.substr(i + 1, 2).c_str(), "%x", &value) == 1) {
                decoded += static_cast<char>(value);
                i += 2;
            } else {
                decoded += encoded[i];
            }
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
}

// Get canonical absolute path safely
std::string get_canonical_path(const std::string& path) {
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved) == nullptr) {
        throw std::runtime_error("Failed to resolve path");
    }
    return std::string(resolved);
}

// Check if path is within base directory
bool is_path_within_base(const std::string& path, const std::string& base) {
    return path.compare(0, base.length(), base) == 0 &&
           (path.length() == base.length() || path[base.length()] == '/');
}

// Main img() function: opens and returns file content
std::vector<char> img(const std::string& name) {
    // Input validation: reject empty or suspicious names
    if (name.empty()) {
        throw std::invalid_argument("File name cannot be empty");
    }
    
    // Decode URL-encoded input
    std::string decoded_name = url_decode(name);
    
    // Sanitize: reject path traversal attempts
    if (decoded_name.find("..") != std::string::npos ||
        decoded_name.find('/') != std::string::npos ||
        decoded_name.find('\\\\') != std::string::npos ||\n        decoded_name[0] == '.') {\n        throw std::runtime_error("Invalid file name: path traversal not allowed");\n    }\n    \n    // Get canonical base directory path\n    std::string canonical_base;\n    try {\n        canonical_base = get_canonical_path(BASE_DIR);\n    } catch (...) {\n        // If base directory doesn't exist, create it first
        mkdir(BASE_DIR.c_str(), 0755);
        canonical_base = get_canonical_path(BASE_DIR);
    }
    
    // Construct requested path
    std::string requested_path = BASE_DIR + "/" + decoded_name;
    
    // Open file with O_RDONLY | O_NOFOLLOW | O_CLOEXEC (no symlink following)
    int fd = open(requested_path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        if (errno == ENOENT) {
            throw std::runtime_error("File not found");
        } else if (errno == ELOOP) {
            throw std::runtime_error("Symlink not allowed");
        }
        throw std::runtime_error("Failed to open file");
    }
    
    // RAII-style cleanup using goto pattern
    std::vector<char> content;
    struct stat st;
    
    // Validate file attributes after opening (TOCTOU prevention)
    if (fstat(fd, &st) != 0) {
        close(fd);
        throw std::runtime_error("Failed to stat file");
    }
    
    // Reject if not a regular file
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        throw std::runtime_error("Not a regular file");
    }
    
    // Check file size limit
    if (st.st_size > static_cast<off_t>(MAX_FILE_SIZE) || st.st_size < 0) {
        close(fd);
        throw std::runtime_error("File size exceeds limit or is invalid");
    }
    
    // Verify resolved path is within base directory
    char resolved_path[PATH_MAX];
    std::string fd_path = "/proc/self/fd/" + std::to_string(fd);
    ssize_t len = readlink(fd_path.c_str(), resolved_path, sizeof(resolved_path) - 1);
    if (len > 0) {
        resolved_path[len] = '\\0';
        if (!is_path_within_base(std::string(resolved_path), canonical_base)) {
            close(fd);
            throw std::runtime_error("Access denied: path outside base directory");
        }
    }
    
    // Read file content with bounded buffer
    content.resize(st.st_size);
    ssize_t total_read = 0;
    ssize_t bytes_read;
    
    while (total_read < st.st_size) {
        bytes_read = read(fd, content.data() + total_read, 
                         st.st_size - total_read);
        if (bytes_read < 0) {
            close(fd);
            throw std::runtime_error("Failed to read file");
        }
        if (bytes_read == 0) {
            break;
        }
        total_read += bytes_read;
    }
    
    close(fd);
    content.resize(total_read);
    return content;
}

int main() {
    // Create test directory
    mkdir(BASE_DIR.c_str(), 0755);
    
    // Create test files
    std::ofstream(BASE_DIR + "/test1.txt") << "Test file 1";
    std::ofstream(BASE_DIR + "/test2.jpg") << "Image data";
    std::ofstream(BASE_DIR + "/safe.png") << "Safe image";
    
    // Test case 1: Valid file
    try {
        auto content = img("test1.txt");
        std::cout << "Test 1 passed: " 
                  << std::string(content.begin(), content.end()) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Another valid file
    try {
        auto content = img("test2.jpg");
        std::cout << "Test 2 passed: " 
                  << std::string(content.begin(), content.end()) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Path traversal attempt (should fail)
    try {
        auto content = img("../etc/passwd");
        std::cout << "Test 3 FAILED: Path traversal succeeded" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 passed: Path traversal blocked" << std::endl;
    }
    
    // Test case 4: Empty input (should fail)
    try {
        auto content = img("");
        std::cout << "Test 4 FAILED: Empty string accepted" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 passed: Empty string rejected" << std::endl;
    }
    
    // Test case 5: Non-existent file (should fail)
    try {
        auto content = img("nonexistent.txt");
        std::cout << "Test 5 FAILED: Non-existent file returned data" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 passed: Non-existent file handled" << std::endl;
    }
    
    return 0;
}
