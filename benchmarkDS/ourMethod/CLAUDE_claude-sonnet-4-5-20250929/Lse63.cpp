
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
const char* BASE_DIR = ".";

// Explicit zero function to prevent compiler optimization
void explicit_zero(void* ptr, size_t len) {
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) {
        *p++ = 0;
    }
}

// Normalize and validate path to prevent traversal
bool validate_path(const char* base, const char* input, char* resolved, size_t resolved_size) {
    if (!base || !input || !resolved) return false;
    
    char base_real[PATH_MAX] = {0};
    char input_full[PATH_MAX] = {0};
    
    // Resolve base directory
    if (!realpath(base, base_real)) {
        return false;
    }
    
    // Build full input path
    int ret = snprintf(input_full, sizeof(input_full), "%s/%s", base_real, input);
    if (ret < 0 || ret >= (int)sizeof(input_full)) {
        return false;
    }
    
    // Resolve input path
    if (!realpath(input_full, resolved)) {
        // If file doesn't exist yet, resolve parent and append filename\n        char parent[PATH_MAX] = {0};\n        char filename[PATH_MAX] = {0};\n        \n        // Find last slash\n        const char* last_slash = strrchr(input_full, '/');\n        if (!last_slash) return false;\n        \n        size_t parent_len = last_slash - input_full;\n        if (parent_len >= sizeof(parent)) return false;\n        \n        memcpy(parent, input_full, parent_len);\n        parent[parent_len] = '\\0';\n        \n        strncpy(filename, last_slash + 1, sizeof(filename) - 1);\n        filename[sizeof(filename) - 1] = '\\0';
        
        char parent_real[PATH_MAX] = {0};
        if (!realpath(parent, parent_real)) {
            return false;
        }
        
        ret = snprintf(resolved, resolved_size, "%s/%s", parent_real, filename);
        if (ret < 0 || ret >= (int)resolved_size) {
            return false;
        }
    }
    
    // Verify resolved path is within base directory
    size_t base_len = strlen(base_real);
    if (strncmp(resolved, base_real, base_len) != 0) {
        return false;
    }
    
    return true;
}

bool process_file(const char* input_path, const char* output_path) {
    if (!input_path || !output_path) {
        std::cerr << "Error: Invalid parameters" << std::endl;
        return false;
    }
    
    char resolved_input[PATH_MAX] = {0};
    char resolved_output[PATH_MAX] = {0};
    
    // Validate and normalize paths
    if (!validate_path(BASE_DIR, input_path, resolved_input, sizeof(resolved_input)) ||
        !validate_path(BASE_DIR, output_path, resolved_output, sizeof(resolved_output))) {
        std::cerr << "Error: Path validation failed" << std::endl;
        return false;
    }
    
    unsigned char* buffer = nullptr;
    size_t bytes_read = 0;
    unsigned int checksum = 0;
    
    // Open file with O_RDONLY, O_CLOEXEC, and O_NOFOLLOW
    int fd = open(resolved_input, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        std::cerr << "Error: Failed to open input file" << std::endl;
        return false;
    }
    
    // Use fstat on opened descriptor to validate file
    struct stat st;
    if (fstat(fd, &st) != 0) {
        std::cerr << "Error: Failed to stat file" << std::endl;
        close(fd);
        return false;
    }
    
    // Check if regular file
    if (!S_ISREG(st.st_mode)) {
        std::cerr << "Error: Not a regular file" << std::endl;
        close(fd);
        return false;
    }
    
    // Check file size limit
    if (st.st_size > (off_t)MAX_FILE_SIZE) {
        std::cerr << "Error: File exceeds maximum size" << std::endl;
        close(fd);
        return false;
    }
    
    // Safe allocation with checked size
    size_t file_size = static_cast<size_t>(st.st_size);
    buffer = new (std::nothrow) unsigned char[file_size];
    if (!buffer) {
        std::cerr << "Error: Memory allocation failed" << std::endl;
        close(fd);
        return false;
    }
    
    // Read file content with bounds checking
    ssize_t total_read = 0;
    while (total_read < (ssize_t)file_size) {
        ssize_t n = read(fd, buffer + total_read, file_size - total_read);
        if (n < 0) {
            if (errno == EINTR) continue;
            std::cerr << "Error: Read failed" << std::endl;
            explicit_zero(buffer, file_size);
            delete[] buffer;
            close(fd);
            return false;
        }
        if (n == 0) break;
        total_read += n;
    }
    close(fd);
    bytes_read = total_read;
    
    // Calculate simple checksum
    for (size_t i = 0; i < bytes_read; i++) {
        checksum = (checksum + buffer[i]) & 0xFF;
    }
    
    // Write result to temp file then atomic rename
    char temp_output[PATH_MAX] = {0};
    snprintf(temp_output, sizeof(temp_output), "%s.tmp%d", resolved_output, getpid());
    
    int out_fd = open(temp_output, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (out_fd < 0) {
        std::cerr << "Error: Failed to create output file" << std::endl;
        explicit_zero(buffer, file_size);
        delete[] buffer;
        return false;
    }
    
    char result[256] = {0};
    int len = snprintf(result, sizeof(result), 
        "Bytes read: %zu\\nChecksum: %u\\nBuffer empty: %s\\n",
        bytes_read, checksum, (checksum == 0 ? "true" : "false"));
    
    if (len > 0 && len < (int)sizeof(result)) {
        ssize_t written = write(out_fd, result, len);
        if (written != len) {
            std::cerr << "Error: Write failed" << std::endl;
            close(out_fd);
            unlink(temp_output);
            explicit_zero(buffer, file_size);
            delete[] buffer;
            return false;
        }
    }
    
    fsync(out_fd);
    close(out_fd);
    
    // Atomic rename
    if (rename(temp_output, resolved_output) != 0) {
        std::cerr << "Error: Rename failed" << std::endl;
        unlink(temp_output);
        explicit_zero(buffer, file_size);
        delete[] buffer;
        return false;
    }
    
    // Clear sensitive data
    explicit_zero(buffer, file_size);
    delete[] buffer;
    
    return true;
}

int main() {
    // Test case 1: Normal file
    {
        std::ofstream ofs("test1.txt");
        ofs << "Hello World";
        ofs.close();
        std::cout << "Test 1: " << (process_file("test1.txt", "output1.txt") ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 2: Empty file
    {
        std::ofstream ofs("test2.txt");
        ofs.close();
        std::cout << "Test 2: " << (process_file("test2.txt", "output2.txt") ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 3: File with zero checksum
    {
        std::ofstream ofs("test3.txt", std::ios::binary);
        char zeros[3] = {0, 0, 0};
        ofs.write(zeros, 3);
        ofs.close();
        std::cout << "Test 3: " << (process_file("test3.txt", "output3.txt") ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 4: Large content
    {
        std::ofstream ofs("test4.txt", std::ios::binary);
        for (int i = 0; i < 1000; i++) {
            char c = i % 256;
            ofs.write(&c, 1);
        }
        ofs.close();
        std::cout << "Test 4: " << (process_file("test4.txt", "output4.txt") ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 5: Path traversal attempt (should fail)
    std::cout << "Test 5: " << (!process_file("../test5.txt", "output5.txt") ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}
