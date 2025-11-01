
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <archive.h>
#include <archive_entry.h>

// Maximum allowed file size: 100 MB to prevent zip bombs
const long MAX_FILE_SIZE = 100 * 1024 * 1024;
// Maximum number of files to prevent excessive resource usage
const int MAX_FILES = 10000;
// Base directory for extraction
const char* BASE_DIR = "/tmp/unpack";

// Helper function to create directories recursively with error checking
bool createDirectories(const char* path) {
    if (!path || strlen(path) == 0) return false;
    
    char tmp[PATH_MAX];
    char* p = nullptr;
    size_t len;
    
    // Copy path safely with bounds checking
    len = strlen(path);
    if (len >= sizeof(tmp)) return false;
    memcpy(tmp, path, len);
    tmp[len] = '\\0';
    
    // Remove trailing slash if present
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\\0';
    }
    
    // Create directories iteratively
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\\0';
            if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
                return false;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
        return false;
    }
    
    return true;
}

// Helper function to normalize and validate paths
bool isPathSafe(const char* basePath, const char* entryPath, char* resolvedPath, size_t resolvedSize) {
    if (!basePath || !entryPath || !resolvedPath) return false;
    
    // Build full path with bounds checking
    char fullPath[PATH_MAX];
    int ret = snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, entryPath);
    if (ret < 0 || (size_t)ret >= sizeof(fullPath)) {
        return false;
    }
    
    // Resolve to absolute path
    if (realpath(fullPath, resolvedPath) == nullptr) {
        // If file doesn't exist yet, resolve parent and append filename\n        char parentPath[PATH_MAX];\n        char* lastSlash = strrchr(fullPath, '/');\n        if (!lastSlash) return false;\n        \n        size_t parentLen = lastSlash - fullPath;\n        if (parentLen >= sizeof(parentPath)) return false;\n        \n        memcpy(parentPath, fullPath, parentLen);\n        parentPath[parentLen] = '\\0';\n        \n        char resolvedParent[PATH_MAX];\n        if (realpath(parentPath, resolvedParent) == nullptr) {\n            // Create parent directory if it doesn't exist
            if (!createDirectories(parentPath)) return false;
            if (realpath(parentPath, resolvedParent) == nullptr) return false;
        }
        
        ret = snprintf(resolvedPath, resolvedSize, "%s%s", resolvedParent, lastSlash);
        if (ret < 0 || (size_t)ret >= resolvedSize) return false;
    }
    
    // Check if resolved path starts with base directory (prevent directory traversal)
    char resolvedBase[PATH_MAX];
    if (realpath(basePath, resolvedBase) == nullptr) {
        return false;
    }
    
    size_t baseLen = strlen(resolvedBase);
    if (strncmp(resolvedPath, resolvedBase, baseLen) != 0) {
        return false;
    }
    
    return true;
}

bool extractZipArchive(const char* zipFilePath) {
    // Validate input
    if (!zipFilePath || strlen(zipFilePath) == 0) {
        std::cerr << "Zip file path cannot be null or empty" << std::endl;
        return false;
    }
    
    // Create base directory
    if (!createDirectories(BASE_DIR)) {
        std::cerr << "Failed to create base directory" << std::endl;
        return false;
    }
    
    // Initialize libarchive for reading
    struct archive* a = archive_read_new();
    if (!a) {
        std::cerr << "Failed to create archive reader" << std::endl;
        return false;
    }
    
    // Support zip format
    archive_read_support_format_zip(a);
    archive_read_support_filter_all(a);
    
    // Open the archive file
    int r = archive_read_open_filename(a, zipFilePath, 10240);
    if (r != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << std::endl;
        archive_read_free(a);
        return false;
    }
    
    struct archive_entry* entry = nullptr;
    int fileCount = 0;
    long totalSize = 0;
    
    // Extract all entries
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        fileCount++;
        
        // Prevent zip bomb by limiting file count
        if (fileCount > MAX_FILES) {
            std::cerr << "Too many files in archive" << std::endl;
            archive_read_free(a);
            return false;
        }
        
        const char* entryName = archive_entry_pathname(entry);
        if (!entryName || strlen(entryName) == 0) {
            archive_read_data_skip(a);
            continue;
        }
        
        // Validate and resolve path
        char resolvedPath[PATH_MAX];
        if (!isPathSafe(BASE_DIR, entryName, resolvedPath, sizeof(resolvedPath))) {
            std::cerr << "Entry is outside target directory: " << entryName << std::endl;
            archive_read_free(a);
            return false;
        }
        
        // Get file size
        la_int64_t size = archive_entry_size(entry);
        if (size > MAX_FILE_SIZE) {
            std::cerr << "File too large: " << entryName << std::endl;
            archive_read_free(a);
            return false;
        }
        
        totalSize += size;
        if (totalSize > MAX_FILE_SIZE * 10) {
            std::cerr << "Total extracted size exceeds limit" << std::endl;
            archive_read_free(a);
            return false;
        }
        
        // Handle directories
        if (archive_entry_filetype(entry) == AE_IFDIR) {
            mkdir(resolvedPath, 0700);
        } else {
            // Extract regular file
            int fd = open(resolvedPath, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
            if (fd < 0) {
                std::cerr << "Failed to create file: " << resolvedPath << std::endl;
                archive_read_free(a);
                return false;
            }
            
            const void* buff;
            size_t buffSize;
            la_int64_t offset;
            
            while (true) {
                r = archive_read_data_block(a, &buff, &buffSize, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r != ARCHIVE_OK) {
                    std::cerr << "Read error: " << archive_error_string(a) << std::endl;
                    close(fd);
                    archive_read_free(a);
                    return false;
                }
                
                ssize_t written = write(fd, buff, buffSize);
                if (written != (ssize_t)buffSize) {
                    std::cerr << "Write error" << std::endl;
                    close(fd);
                    archive_read_free(a);
                    return false;
                }
            }
            
            // Ensure data is written to disk
            fsync(fd);
            close(fd);
        }
    }
    
    archive_read_free(a);
    return true;
}

int main() {
    // Test case 1: Valid zip file
    std::cout << "Test 1: Extracting valid archive" << std::endl;
    if (extractZipArchive("archive.zip")) {
        std::cout << "Test 1 passed" << std::endl;
    } else {
        std::cout << "Test 1 failed" << std::endl;
    }
    
    // Test case 2: Non-existent file
    std::cout << "\\nTest 2: Non-existent file" << std::endl;
    if (!extractZipArchive("nonexistent.zip")) {
        std::cout << "Test 2 passed" << std::endl;
    } else {
        std::cout << "Test 2 failed" << std::endl;
    }
    
    // Test case 3: Null input
    std::cout << "\\nTest 3: Null input" << std::endl;
    if (!extractZipArchive(nullptr)) {
        std::cout << "Test 3 passed" << std::endl;
    } else {
        std::cout << "Test 3 failed" << std::endl;
    }
    
    // Test case 4: Empty string input
    std::cout << "\\nTest 4: Empty string input" << std::endl;
    if (!extractZipArchive("")) {
        std::cout << "Test 4 passed" << std::endl;
    } else {
        std::cout << "Test 4 failed" << std::endl;
    }
    
    // Test case 5: Path with directory traversal attempt
    std::cout << "\\nTest 5: Malicious path (if archive contains traversal)" << std::endl;
    if (extractZipArchive("test_traversal.zip")) {
        std::cout << "Test 5: extraction completed with validation" << std::endl;
    } else {
        std::cout << "Test 5: rejected malicious path" << std::endl;
    }
    
    return 0;
}
