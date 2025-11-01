
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

// Note: Using minizip library (part of zlib contrib) for zip extraction
// Link with: -lz -lminizip
#include <minizip/unzip.h>

// Maximum file size to prevent zip bomb attacks (100MB)
#define MAX_FILE_SIZE (100LL * 1024 * 1024)
// Maximum total extracted size (500MB)
#define MAX_TOTAL_SIZE (500LL * 1024 * 1024)
// Maximum number of files
#define MAX_FILE_COUNT 10000
// Base directory
#define BASE_DIR "/tmp/unpack"
// Buffer size for extraction
#define BUFFER_SIZE 8192

// Create directory recursively with secure permissions
static int create_directory_recursive(const char* path) {
    if (!path || path[0] == '\\0') {
        return -1;
    }
    
    char tmp[PATH_MAX];
    char* p = NULL;
    size_t len;
    
    // Copy path and validate length
    len = strlen(path);
    if (len >= PATH_MAX) {
        return -1;
    }
    snprintf(tmp, sizeof(tmp), "%s", path);
    
    // Remove trailing slash if present
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\\0';
    }
    
    // Create directories along the path
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\\0';
            // Create directory with restricted permissions (0700)
            if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    // Create final directory
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
        return -1;
    }
    
    return 0;
}

// Validate path to prevent directory traversal
static int validate_path(const char* base_dir, const char* entry_name, char* output_path, size_t output_size) {
    if (!base_dir || !entry_name || !output_path) {
        return -1;
    }
    
    // Build the full path
    if (snprintf(output_path, output_size, "%s/%s", base_dir, entry_name) >= (int)output_size) {
        return -1;  // Path too long
    }
    
    // Resolve to absolute path to detect traversal
    char resolved[PATH_MAX];
    if (realpath(base_dir, resolved) == NULL) {
        // If base doesn't exist yet, that's ok - we'll create it\n        strncpy(resolved, base_dir, PATH_MAX - 1);\n        resolved[PATH_MAX - 1] = '\\0';\n    }\n    \n    size_t base_len = strlen(resolved);\n    \n    // Check if the constructed path starts with base directory\n    // This prevents "../" style attacks\n    if (strstr(entry_name, "..") != NULL) {\n        return -1;  // Potential traversal attempt\n    }\n    \n    // Additional check: ensure no absolute paths in entry name\n    if (entry_name[0] == '/') {\n        return -1;\n    }\n    \n    return 0;\n}\n\nint unzip_archive(const char* zip_file_path) {\n    if (!zip_file_path || zip_file_path[0] == '\\0') {\n        std::cerr << "Invalid zip file path" << std::endl;\n        return -1;\n    }\n    \n    // Create base directory\n    if (create_directory_recursive(BASE_DIR) != 0) {\n        std::cerr << "Failed to create base directory" << std::endl;\n        return -1;\n    }\n    \n    // Open zip file\n    unzFile uf = unzOpen(zip_file_path);\n    if (uf == NULL) {\n        std::cerr << "Cannot open zip file: " << zip_file_path << std::endl;\n        return -1;\n    }\n    \n    long long total_size = 0;\n    int file_count = 0;\n    int ret = 0;\n    \n    // Get global info\n    unz_global_info gi;\n    if (unzGetGlobalInfo(uf, &gi) != UNZ_OK) {\n        unzClose(uf);\n        std::cerr << "Failed to get zip file info" << std::endl;\n        return -1;\n    }\n    \n    // Process each file in archive\n    for (uLong i = 0; i < gi.number_entry; i++) {\n        file_count++;\n        if (file_count > MAX_FILE_COUNT) {\n            std::cerr << "Too many files - potential zip bomb" << std::endl;\n            ret = -1;\n            break;\n        }\n        \n        char filename[PATH_MAX];\n        unz_file_info file_info;\n        \n        // Get file info\n        if (unzGetCurrentFileInfo(uf, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK) {\n            ret = -1;\n            break;\n        }\n        \n        // Validate and construct output path\n        char output_path[PATH_MAX];\n        if (validate_path(BASE_DIR, filename, output_path, sizeof(output_path)) != 0) {\n            std::cerr << "Path traversal attempt detected: " << filename << std::endl;\n            ret = -1;\n            break;\n        }\n        \n        // Check if it's a directory
        size_t filename_len = strlen(filename);
        if (filename_len > 0 && filename[filename_len - 1] == '/') {
            // Create directory
            if (create_directory_recursive(output_path) != 0) {
                ret = -1;
                break;
            }
        } else {
            // Extract file
            // Create parent directory
            char* last_slash = strrchr(output_path, '/');
            if (last_slash != NULL) {
                *last_slash = '\\0';
                create_directory_recursive(output_path);
                *last_slash = '/';
            }
            
            // Open file in zip
            if (unzOpenCurrentFile(uf) != UNZ_OK) {
                ret = -1;
                break;
            }
            
            // Open output file with restricted permissions (0600)
            int fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
            if (fd < 0) {
                unzCloseCurrentFile(uf);
                ret = -1;
                break;
            }
            
            // Extract with size validation
            unsigned char buffer[BUFFER_SIZE];
            long long entry_size = 0;
            int bytes_read;
            
            while ((bytes_read = unzReadCurrentFile(uf, buffer, BUFFER_SIZE)) > 0) {
                entry_size += bytes_read;
                total_size += bytes_read;
                
                // Check size limits to prevent zip bombs
                if (entry_size > MAX_FILE_SIZE) {
                    std::cerr << "File too large - potential zip bomb: " << filename << std::endl;
                    close(fd);
                    unzCloseCurrentFile(uf);
                    ret = -1;
                    break;
                }
                
                if (total_size > MAX_TOTAL_SIZE) {
                    std::cerr << "Total size exceeds limit - potential zip bomb" << std::endl;
                    close(fd);
                    unzCloseCurrentFile(uf);
                    ret = -1;
                    break;
                }
                
                // Write to file
                ssize_t written = write(fd, buffer, bytes_read);
                if (written != bytes_read) {
                    close(fd);
                    unzCloseCurrentFile(uf);
                    ret = -1;
                    break;
                }
            }
            
            // Sync and close
            if (ret == 0) {
                fsync(fd);
            }
            close(fd);
            unzCloseCurrentFile(uf);
            
            if (ret != 0 || bytes_read < 0) {
                ret = -1;
                break;
            }
        }
        
        // Move to next file
        if (i + 1 < gi.number_entry) {
            if (unzGoToNextFile(uf) != UNZ_OK) {
                ret = -1;
                break;
            }
        }
    }
    
    unzClose(uf);
    return ret;
}

int main() {
    // Test case 1: Valid zip file
    std::cout << "Test 1: Extracting valid archive" << std::endl;
    if (unzip_archive("archive.zip") == 0) {
        std::cout << "Test 1: SUCCESS" << std::endl;
    } else {
        std::cout << "Test 1: FAILED" << std::endl;
    }
    
    // Test case 2: Non-existent file
    std::cout << "\\nTest 2: Non-existent file" << std::endl;
    if (unzip_archive("nonexistent.zip") != 0) {
        std::cout << "Test 2: SUCCESS - Correctly failed" << std::endl;
    } else {
        std::cout << "Test 2: FAILED - Should have failed" << std::endl;
    }
    
    // Test case 3: NULL input
    std::cout << "\\nTest 3: NULL input" << std::endl;
    if (unzip_archive(NULL) != 0) {
        std::cout << "Test 3: SUCCESS - Correctly rejected" << std::endl;
    } else {
        std::cout << "Test 3: FAILED - Should have rejected" << std::endl;
    }
    
    // Test case 4: Empty string
    std::cout << "\\nTest 4: Empty string input" << std::endl;
    if (unzip_archive("") != 0) {
        std::cout << "Test 4: SUCCESS - Correctly rejected" << std::endl;
    } else {
        std::cout << "Test 4: FAILED - Should have rejected" << std::endl;
    }
    
    // Test case 5: Special characters in name
    std::cout << "\\nTest 5: Archive with special name" << std::endl;
    if (unzip_archive("test-archive_v1.0.zip") == 0) {
        std::cout << "Test 5: SUCCESS" << std::endl;
    } else {
        std::cout << "Test 5: FAILED" << std::endl;
    }
    
    return 0;
}
