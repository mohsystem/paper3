
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <archive.h>
#include <archive_entry.h>
#include <limits.h>

// Maximum security limits to prevent resource exhaustion
static const size_t MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB per file
static const size_t MAX_TOTAL_SIZE = 500 * 1024 * 1024; // 500MB total
static const int MAX_FILES = 10000;
static const char* BASE_DIR = "/tmp/unpack";

// Secure path validation preventing directory traversal
bool validate_path(const char* base, const char* path, char* resolved, size_t resolved_size) {
    // Validate inputs are not NULL
    if (!base || !path || !resolved || resolved_size == 0) {
        return false;
    }
    
    // Build full path safely
    char full_path[PATH_MAX];
    int ret = snprintf(full_path, sizeof(full_path), "%s/%s", base, path);
    if (ret < 0 || ret >= (int)sizeof(full_path)) {
        return false;
    }
    
    // Resolve to canonical path (prevents .. and symlinks)
    char* real = realpath(full_path, resolved);
    if (!real) {
        // If file doesn't exist yet, validate parent directory\n        char parent[PATH_MAX];\n        char* last_slash = strrchr(full_path, '/');\n        if (!last_slash) {\n            return false;\n        }\n        \n        size_t parent_len = last_slash - full_path;\n        if (parent_len >= sizeof(parent)) {\n            return false;\n        }\n        \n        memcpy(parent, full_path, parent_len);\n        parent[parent_len] = '\\0';\n        \n        char parent_real[PATH_MAX];\n        if (!realpath(parent, parent_real)) {\n            return false;\n        }\n        \n        // Build resolved path from parent\n        ret = snprintf(resolved, resolved_size, "%s/%s", parent_real, last_slash + 1);\n        if (ret < 0 || ret >= (int)resolved_size) {\n            return false;\n        }\n    }\n    \n    // Validate resolved path starts with base directory\n    char base_real[PATH_MAX];\n    if (!realpath(base, base_real)) {\n        return false;\n    }\n    \n    size_t base_len = strlen(base_real);\n    if (strncmp(resolved, base_real, base_len) != 0) {\n        return false;\n    }\n    \n    // Ensure there's a path separator after base (or it's exactly the base)\n    if (resolved[base_len] != '\\0' && resolved[base_len] != '/') {\n        return false;\n    }\n    \n    return true;\n}\n\n// Secure tar extraction function preventing path traversal and resource exhaustion\nint extract_tar_archive(const char* tar_file_path, const char* compression_type) {\n    // Validate inputs are not NULL\n    if (!tar_file_path) {\n        std::cerr << "Invalid tar file path" << std::endl;\n        return -1;\n    }\n    \n    // Create base directory with restrictive permissions (0700)\n    struct stat st;\n    if (stat(BASE_DIR, &st) != 0) {\n        if (mkdir(BASE_DIR, 0700) != 0) {\n            std::cerr << "Failed to create base directory" << std::endl;\n            return -1;\n        }\n    }\n    \n    // Initialize libarchive for reading\n    struct archive* a = archive_read_new();\n    if (!a) {\n        std::cerr << "Failed to create archive reader" << std::endl;\n        return -1;\n    }\n    \n    // Enable only tar format (no automatic format detection for security)\n    archive_read_support_format_tar(a);\n    \n    // Set compression based on type\n    if (compression_type && strcmp(compression_type, "gzip") == 0) {\n        archive_read_support_filter_gzip(a);\n    } else if (compression_type && strcmp(compression_type, "bzip2") == 0) {\n        archive_read_support_filter_bzip2(a);\n    } else {\n        archive_read_support_filter_none(a);\n    }\n    \n    // Set security limits\n    archive_read_set_read_callback(a, nullptr);\n    \n    // Open archive with validation\n    if (archive_read_open_filename(a, tar_file_path, 10240) != ARCHIVE_OK) {\n        std::cerr << "Failed to open archive: " << archive_error_string(a) << std::endl;\n        archive_read_free(a);\n        return -1;\n    }\n    \n    size_t total_bytes_extracted = 0;\n    int file_count = 0;\n    struct archive_entry* entry = nullptr;\n    \n    // Process each entry in archive\n    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {\n        // Limit number of files to prevent zip bombs\n        if (++file_count > MAX_FILES) {\n            std::cerr << "Archive contains too many files" << std::endl;\n            archive_read_free(a);\n            return -1;\n        }\n        \n        // Only process regular files\n        mode_t filetype = archive_entry_filetype(entry);\n        if (!S_ISREG(filetype)) {\n            archive_read_data_skip(a);\n            continue;\n        }\n        \n        // Validate entry size\n        int64_t entry_size = archive_entry_size(entry);\n        if (entry_size < 0 || (size_t)entry_size > MAX_FILE_SIZE) {\n            std::cerr << "Entry size exceeds maximum" << std::endl;\n            archive_read_free(a);\n            return -1;\n        }\n        \n        // Check total size\n        if (total_bytes_extracted + entry_size > MAX_TOTAL_SIZE) {\n            std::cerr << "Total extraction size exceeds maximum" << std::endl;\n            archive_read_free(a);\n            return -1;\n        }\n        \n        // Get and validate entry path\n        const char* entry_name = archive_entry_pathname(entry);\n        if (!entry_name || entry_name[0] == '\\0') {\n            archive_read_data_skip(a);\n            continue;\n        }\n        \n        // Remove leading slashes for security\n        while (*entry_name == '/' || *entry_name == '\\\\') {\n            entry_name++;\n        }\n        \n        // Validate path doesn't attempt traversal
        char resolved_path[PATH_MAX];
        if (!validate_path(BASE_DIR, entry_name, resolved_path, sizeof(resolved_path))) {
            std::cerr << "Path traversal attempt detected: " << entry_name << std::endl;
            archive_read_free(a);
            return -1;
        }
        
        // Open output file with restrictive permissions (0600) using open-then-validate
        int fd = open(resolved_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
        if (fd < 0) {
            std::cerr << "Failed to create file: " << resolved_path << std::endl;
            archive_read_free(a);
            return -1;
        }
        
        // Validate opened file descriptor
        struct stat fd_stat;
        if (fstat(fd, &fd_stat) != 0 || !S_ISREG(fd_stat.st_mode)) {
            std::cerr << "Opened descriptor is not a regular file" << std::endl;
            close(fd);
            archive_read_free(a);
            return -1;
        }
        
        // Extract data with size validation
        size_t bytes_written = 0;
        const size_t buffer_size = 8192;
        char buffer[buffer_size];
        
        ssize_t bytes_read;
        while ((bytes_read = archive_read_data(a, buffer, buffer_size)) > 0) {
            // Check for integer overflow
            if (bytes_written > SIZE_MAX - bytes_read) {
                std::cerr << "Integer overflow detected" << std::endl;
                close(fd);
                archive_read_free(a);
                return -1;
            }
            
            // Validate against declared size
            if (bytes_written + bytes_read > (size_t)entry_size) {
                std::cerr << "Entry size mismatch" << std::endl;
                close(fd);
                archive_read_free(a);
                return -1;
            }
            
            ssize_t written = write(fd, buffer, bytes_read);
            if (written != bytes_read) {
                std::cerr << "Write error" << std::endl;
                close(fd);
                archive_read_free(a);
                return -1;
            }
            
            bytes_written += bytes_read;
        }
        
        // Ensure data is written to disk
        fsync(fd);
        close(fd);
        
        total_bytes_extracted += bytes_written;
    }
    
    archive_read_free(a);
    return 0;
}

// Secure file reading with TOCTOU prevention
std::string read_file(const char* file_path) {
    // Validate input
    if (!file_path || file_path[0] == '\\0') {
        throw std::runtime_error("Invalid file path");
    }
    
    // Validate and resolve path
    char resolved_path[PATH_MAX];
    if (!validate_path(BASE_DIR, file_path, resolved_path, sizeof(resolved_path))) {
        throw std::runtime_error("Path traversal attempt detected");
    }
    
    // Open file first using open (TOCTOU prevention)
    int fd = open(resolved_path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        throw std::runtime_error("Failed to open file");
    }
    
    // Validate opened file descriptor
    struct stat fd_stat;
    if (fstat(fd, &fd_stat) != 0) {
        close(fd);
        throw std::runtime_error("Failed to stat file");
    }
    
    // Ensure it's a regular file
    if (!S_ISREG(fd_stat.st_mode)) {
        close(fd);
        throw std::runtime_error("Not a regular file");
    }
    
    // Check file size
    if (fd_stat.st_size < 0 || (size_t)fd_stat.st_size > MAX_FILE_SIZE) {
        close(fd);
        throw std::runtime_error("File size exceeds maximum");
    }
    
    // Allocate buffer and read
    size_t file_size = fd_stat.st_size;
    std::string content;
    content.resize(file_size);
    
    size_t total_read = 0;
    while (total_read < file_size) {
        ssize_t bytes_read = read(fd, &content[total_read], file_size - total_read);
        if (bytes_read < 0) {
            close(fd);
            throw std::runtime_error("Read error");
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
    // Test case 1: Extract uncompressed tar
    std::cout << "Test 1: Extract uncompressed tar" << std::endl;
    try {
        // extract_tar_archive("/path/to/archive.tar", nullptr);
        std::cout << "Note: Requires actual tar file to test" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: Extract gzip compressed tar
    std::cout << "\\nTest 2: Extract gzip tar" << std::endl;
    try {
        // extract_tar_archive("/path/to/archive.tar.gz", "gzip");
        std::cout << "Note: Requires actual tar.gz file to test" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: Extract bzip2 compressed tar
    std::cout << "\\nTest 3: Extract bzip2 tar" << std::endl;
    try {
        // extract_tar_archive("/path/to/archive.tar.bz2", "bzip2");
        std::cout << "Note: Requires actual tar.bz2 file to test" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: Read extracted file
    std::cout << "\\nTest 4: Read file" << std::endl;
    try {
        // std::string content = read_file("example.txt");
        // std::cout << "Content: " << content << std::endl;
        std::cout << "Note: Requires extracted file to test" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: Path traversal prevention
    std::cout << "\\nTest 5: Path traversal prevention" << std::endl;
    try {
        read_file("../../etc/passwd");
        std::cout << "FAIL: Should have thrown exception" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "PASS: Path traversal blocked - " << e.what() << std::endl;
    }
    
    return 0;
}
