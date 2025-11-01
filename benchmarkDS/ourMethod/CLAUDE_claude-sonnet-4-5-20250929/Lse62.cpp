
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <limits.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)  // 10MB limit
#define BUFFER_SIZE 8192

// Secure memory clearing function
void secure_zero(void* ptr, size_t size) {
    if (ptr == nullptr || size == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (size--) {
        *p++ = 0;
    }
}

struct FileResult {
    ssize_t bytes_read;
    std::string checksum;
    bool is_empty;
};

// Convert bytes to hex string
std::string bytes_to_hex(const unsigned char* data, size_t len) {
    const char hex_chars[] = "0123456789abcdef";
    std::string result;
    result.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        result.push_back(hex_chars[(data[i] >> 4) & 0x0F]);
        result.push_back(hex_chars[data[i] & 0x0F]);
    }
    return result;
}

FileResult process_file(const std::string& base_dir, const std::string& input_rel_path, 
                        const std::string& output_rel_path) {
    FileResult result = {-1, "", false};
    
    // Resolve base directory
    char resolved_base[PATH_MAX] = {0};
    if (realpath(base_dir.c_str(), resolved_base) == nullptr) {
        throw std::runtime_error("Invalid base directory");
    }
    
    // Construct and validate input path
    std::string input_full = std::string(resolved_base) + "/" + input_rel_path;
    char resolved_input[PATH_MAX] = {0};
    if (realpath(input_full.c_str(), resolved_input) == nullptr) {
        throw std::runtime_error("Cannot resolve input path");
    }
    
    // Ensure input path is within base directory
    if (strncmp(resolved_input, resolved_base, strlen(resolved_base)) != 0) {
        throw std::runtime_error("Path traversal attempt detected");
    }
    
    // Open file with O_NOFOLLOW to prevent symlink following
    int fd = open(resolved_input, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd == -1) {
        throw std::runtime_error("Cannot open input file");
    }
    
    // Validate file descriptor
    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        throw std::runtime_error("Cannot stat input file");
    }
    
    // Ensure it's a regular file\n    if (!S_ISREG(st.st_mode)) {\n        close(fd);\n        throw std::runtime_error("Input must be a regular file");\n    }\n    \n    // Check file size\n    if (st.st_size > MAX_FILE_SIZE) {\n        close(fd);\n        throw std::runtime_error("File size exceeds maximum allowed");\n    }\n    \n    // Allocate buffer\n    std::vector<unsigned char> buffer(st.st_size);\n    ssize_t bytes_read = 0;\n    ssize_t total_read = 0;\n    \n    // Read file contents with bounds checking\n    while (total_read < st.st_size) {\n        bytes_read = read(fd, buffer.data() + total_read, st.st_size - total_read);\n        if (bytes_read == -1) {\n            close(fd);\n            secure_zero(buffer.data(), buffer.size());\n            throw std::runtime_error("Read error");\n        }\n        if (bytes_read == 0) break;\n        total_read += bytes_read;\n    }\n    \n    close(fd);\n    \n    // Calculate SHA-256 checksum\n    unsigned char hash[SHA256_DIGEST_LENGTH];\n    SHA256(buffer.data(), total_read, hash);\n    \n    // Check if checksum is all zeros\n    bool is_empty = true;\n    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {\n        if (hash[i] != 0) {\n            is_empty = false;\n            break;\n        }\n    }\n    \n    std::string checksum_hex = bytes_to_hex(hash, SHA256_DIGEST_LENGTH);\n    \n    // Construct and validate output path\n    std::string output_full = std::string(resolved_base) + "/" + output_rel_path;\n    char resolved_output[PATH_MAX] = {0};\n    \n    // Get parent directory for output\n    std::string output_dir = output_full.substr(0, output_full.find_last_of('/'));\n    \n    // Create temp file in same directory\n    std::string temp_template = output_dir + "/tmpXXXXXX";\n    std::vector<char> temp_path(temp_template.begin(), temp_template.end());\n    temp_path.push_back('\\0');
    
    int temp_fd = mkstemp(temp_path.data());
    if (temp_fd == -1) {
        secure_zero(buffer.data(), buffer.size());
        throw std::runtime_error("Cannot create temp file");
    }
    
    // Write results to temp file
    std::string output_content = "Bytes Read: " + std::to_string(total_read) + "\\n";
    output_content += "Checksum: " + checksum_hex + "\\n";
    output_content += "Buffer Empty: " + std::string(is_empty ? "true" : "false") + "\\n";
    
    if (write(temp_fd, output_content.c_str(), output_content.length()) == -1) {
        close(temp_fd);
        unlink(temp_path.data());
        secure_zero(buffer.data(), buffer.size());
        throw std::runtime_error("Write error");
    }
    
    fsync(temp_fd);
    close(temp_fd);
    
    // Atomic rename
    if (rename(temp_path.data(), output_full.c_str()) == -1) {
        unlink(temp_path.data());
        secure_zero(buffer.data(), buffer.size());
        throw std::runtime_error("Cannot rename output file");
    }
    
    // Clear sensitive data
    secure_zero(buffer.data(), buffer.size());
    
    result.bytes_read = total_read;
    result.checksum = checksum_hex;
    result.is_empty = is_empty;
    
    return result;
}

int main() {
    const char* base_dir = "/tmp";
    
    try {
        // Test case 1: Normal file with content
        {
            std::ofstream f("/tmp/test1.txt");
            f << "Hello World";
            f.close();
            FileResult r = process_file(base_dir, "test1.txt", "output1.txt");
            std::cout << "Test 1 - Bytes: " << r.bytes_read << ", Empty: " << r.is_empty << std::endl;
        }
        
        // Test case 2: Empty file
        {
            std::ofstream f("/tmp/test2.txt");
            f.close();
            FileResult r = process_file(base_dir, "test2.txt", "output2.txt");
            std::cout << "Test 2 - Bytes: " << r.bytes_read << ", Empty: " << r.is_empty << std::endl;
        }
        
        // Test case 3: File with binary data
        {
            std::ofstream f("/tmp/test3.bin", std::ios::binary);
            unsigned char data[] = {1, 2, 3, 4, 5};
            f.write(reinterpret_cast<char*>(data), sizeof(data));
            f.close();
            FileResult r = process_file(base_dir, "test3.bin", "output3.txt");
            std::cout << "Test 3 - Bytes: " << r.bytes_read << ", Empty: " << r.is_empty << std::endl;
        }
        
        // Test case 4: Larger file
        {
            std::ofstream f("/tmp/test4.txt", std::ios::binary);
            for (int i = 0; i < 1024; ++i) {
                unsigned char byte = i % 256;
                f.write(reinterpret_cast<char*>(&byte), 1);
            }
            f.close();
            FileResult r = process_file(base_dir, "test4.txt", "output4.txt");
            std::cout << "Test 4 - Bytes: " << r.bytes_read << ", Empty: " << r.is_empty << std::endl;
        }
        
        // Test case 5: File with special characters
        {
            std::ofstream f("/tmp/test5.txt");
            f << "Special: \\n\\t\\r";
            f.close();
            FileResult r = process_file(base_dir, "test5.txt", "output5.txt");
            std::cout << "Test 5 - Bytes: " << r.bytes_read << ", Empty: " << r.is_empty << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
