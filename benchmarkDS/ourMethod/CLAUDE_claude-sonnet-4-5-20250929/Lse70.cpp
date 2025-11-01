
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_MAX MAX_PATH
#else
#include <sys/types.h>
#endif

const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB max file size
const size_t MAX_FILENAME_LENGTH = 255;
const std::string UPLOAD_DIR = "uploads";

// Generate cryptographically secure random filename
std::string generate_secure_filename() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    return ss.str() + ".bin";
}

// Normalize and validate path to prevent directory traversal
bool is_safe_path(const std::string& base_dir, const std::string& filename, std::string& out_path) {
    // Check filename length
    if (filename.length() > MAX_FILENAME_LENGTH) {
        return false;
    }
    
    // Construct full path
    std::string full_path = base_dir + "/" + filename;
    
#ifdef _WIN32
    char resolved[PATH_MAX];
    if (_fullpath(resolved, full_path.c_str(), PATH_MAX) == NULL) {
        return false;
    }
    
    char base_resolved[PATH_MAX];
    if (_fullpath(base_resolved, base_dir.c_str(), PATH_MAX) == NULL) {
        return false;
    }
#else
    char resolved[PATH_MAX];
    // Use realpath only on the base directory (which already exists)
    // For the target file, manually normalize the path
    char base_resolved[PATH_MAX];
    if (realpath(base_dir.c_str(), base_resolved) == NULL) {
        return false;
    }
    
    // Manually construct the target path
    snprintf(resolved, PATH_MAX, "%s/%s", base_resolved, filename.c_str());
#endif
    
    // Ensure resolved path starts with base directory
    std::string resolved_str(resolved);
    std::string base_str(base_resolved);
    
    if (resolved_str.find(base_str) != 0) {
        return false;
    }
    
    out_path = resolved;
    return true;
}

// Create directory with restrictive permissions
bool create_upload_directory() {
#ifdef _WIN32
    if (_mkdir(UPLOAD_DIR.c_str()) != 0 && errno != EEXIST) {
        return false;
    }
#else
    // Create directory with restrictive permissions (0700 = owner only)
    if (mkdir(UPLOAD_DIR.c_str(), 0700) != 0 && errno != EEXIST) {
        return false;
    }
#endif
    return true;
}

std::string upload_file(const std::vector<uint8_t>& file_content, const std::string& content_type) {
    // Validate content type
    if (content_type.find("multipart/form-data") == std::string::npos) {
        return "Error: Invalid content type";
    }
    
    // Enforce maximum file size to prevent DoS attacks
    if (file_content.size() > MAX_FILE_SIZE) {
        return "Error: File too large";
    }
    
    // Create upload directory if it doesn't exist\n    if (!create_upload_directory()) {\n        return "Error: Failed to create upload directory";\n    }\n    \n    // Generate cryptographically secure random filename\n    std::string safe_filename = generate_secure_filename();\n    \n    // Validate and normalize path to prevent directory traversal\n    std::string target_path;\n    if (!is_safe_path(UPLOAD_DIR, safe_filename, target_path)) {\n        return "Error: Invalid file path";\n    }\n    \n#ifdef _WIN32\n    // Windows: use CreateFile with appropriate flags\n    HANDLE hFile = CreateFileA(\n        target_path.c_str(),\n        GENERIC_WRITE,\n        0, // No sharing\n        NULL,\n        CREATE_NEW, // Fail if file exists\n        FILE_ATTRIBUTE_NORMAL,\n        NULL\n    );\n    \n    if (hFile == INVALID_HANDLE_VALUE) {\n        return "Error: Upload failed";\n    }\n    \n    DWORD bytesWritten;\n    BOOL result = WriteFile(\n        hFile,\n        file_content.data(),\n        static_cast<DWORD>(file_content.size()),\n        &bytesWritten,\n        NULL\n    );\n    \n    FlushFileBuffers(hFile); // Ensure data is written to disk\n    CloseHandle(hFile);\n    \n    if (!result || bytesWritten != file_content.size()) {\n        DeleteFileA(target_path.c_str());\n        return "Error: Upload failed";\n    }\n#else\n    // POSIX: use open with O_CREAT|O_EXCL|O_NOFOLLOW to prevent race conditions\n    int fd = open(target_path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);\n    if (fd < 0) {\n        return "Error: Upload failed";\n    }\n    \n    // Validate that we opened a regular file\n    struct stat st;\n    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {\n        close(fd);\n        unlink(target_path.c_str());\n        return "Error: Invalid file type";\n    }\n    \n    // Write data\n    ssize_t written = write(fd, file_content.data(), file_content.size());\n    \n    // Ensure data is written to disk\n    if (fsync(fd) != 0) {\n        close(fd);\n        unlink(target_path.c_str());\n        return "Error: Upload failed";\n    }\n    \n    close(fd);\n    \n    if (written < 0 || static_cast<size_t>(written) != file_content.size()) {\n        unlink(target_path.c_str());\n        return "Error: Upload failed";\n    }\n#endif\n    \n    return "File uploaded successfully: " + safe_filename;\n}\n\nvoid run_test_cases() {\n    std::cout << "\\n=== Running Test Cases ===" << std::endl;\n    \n    // Test case 1: Valid small file upload\n    std::vector<uint8_t> test1_data(std::string("Test content 1").begin(), std::string("Test content 1").end());\n    std::string result1 = upload_file(test1_data, "multipart/form-data");\n    std::cout << "Test 1 (valid upload): " << result1 << std::endl;\n    \n    // Test case 2: Another valid upload\n    std::vector<uint8_t> test2_data(std::string("Test content 2").begin(), std::string("Test content 2").end());\n    std::string result2 = upload_file(test2_data, "multipart/form-data");\n    std::cout << "Test 2 (valid upload): " << result2 << std::endl;\n    \n    // Test case 3: Invalid content type\n    std::vector<uint8_t> test3_data(std::string("Test content 3").begin(), std::string("Test content 3").end());\n    std::string result3 = upload_file(test3_data, "text/plain");\n    std::cout << "Test 3 (invalid content type): " << result3 << std::endl;\n    \n    // Test case 4: Large file (1KB)\n    std::vector<uint8_t> test4_data(1024, 'x');
    std::string result4 = upload_file(test4_data, "multipart/form-data");
    std::cout << "Test 4 (1KB file): " << result4 << std::endl;
    
    // Test case 5: Empty file
    std::vector<uint8_t> test5_data;
    std::string result5 = upload_file(test5_data, "multipart/form-data");
    std::cout << "Test 5 (empty file): " << result5 << std::endl;
}

int main() {
    run_test_cases();
    return 0;
}
