
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#endif

// Security: Get secure base directory path
std::string getBaseDirectory() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        return std::string(path) + "\\\\app_data";
    }
    throw std::runtime_error("Failed to get home directory");
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
    if (!home) {
        throw std::runtime_error("Failed to get home directory");
    }
    return std::string(home) + "/app_data";
#endif
}

// Security: Create directory with restrictive permissions
void createSecureDirectory(const std::string& path) {
#ifdef _WIN32
    CreateDirectoryA(path.c_str(), NULL);
    // Windows: Set ACLs for owner-only access would require additional code
#else
    // Security: Create with owner-only permissions (0700)
    mkdir(path.c_str(), S_IRWXU);
#endif
}

// Security: Validate filename to prevent path traversal
bool isValidFilename(const std::string& filename) {
    if (filename.empty()) return false;
    // Security: Reject path traversal patterns
    if (filename.find("..") != std::string::npos) return false;
    if (filename.find("/") != std::string::npos) return false;
    if (filename.find("\\\\") != std::string::npos) return false;\n    return true;\n}\n\n// Security: Write file with atomic operation and restrictive permissions\nvoid writeConfig(const std::string& filename, const std::string& content) {\n    // Security: Input validation\n    if (!isValidFilename(filename)) {\n        throw std::invalid_argument("Invalid filename or path traversal attempt");\n    }\n    \n    std::string baseDir = getBaseDirectory();\n    createSecureDirectory(baseDir);\n    \n    std::string targetPath = baseDir + "/" + filename;\n    std::string tempPath = baseDir + "/.tmp_config_XXXXXX";\n    \n#ifdef _WIN32\n    // Windows implementation using temporary file\n    char tempFile[MAX_PATH];\n    if (GetTempFileNameA(baseDir.c_str(), "tmp", 0, tempFile) == 0) {\n        throw std::runtime_error("Failed to create temporary file");\n    }\n    \n    // Security: Write to temp file with error checking\n    HANDLE hFile = CreateFileA(\n        tempFile,\n        GENERIC_WRITE,\n        0, // No sharing\n        NULL,\n        CREATE_ALWAYS,\n        FILE_ATTRIBUTE_NORMAL,\n        NULL\n    );\n    \n    if (hFile == INVALID_HANDLE_VALUE) {\n        throw std::runtime_error("Failed to open temp file");\n    }\n    \n    DWORD written;\n    if (!WriteFile(hFile, content.c_str(), content.length(), &written, NULL)) {\n        CloseHandle(hFile);\n        DeleteFileA(tempFile);\n        throw std::runtime_error("Failed to write to temp file");\n    }\n    \n    // Security: Flush to disk\n    FlushFileBuffers(hFile);\n    CloseHandle(hFile);\n    \n    // Security: Atomic move\n    if (!MoveFileExA(tempFile, targetPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {\n        DeleteFileA(tempFile);\n        throw std::runtime_error("Failed to move temp file to target");\n    }\n#else\n    // POSIX implementation with secure temporary file\n    char tempTemplate[256];\n    snprintf(tempTemplate, sizeof(tempTemplate), "%s/.tmp_XXXXXX", baseDir.c_str());\n    \n    // Security: Create temp file with owner-only permissions (0600)\n    int fd = mkstemp(tempTemplate);\n    if (fd < 0) {\n        throw std::runtime_error(std::string("Failed to create temp file: ") + strerror(errno));\n    }\n    \n    // Security: Set restrictive permissions immediately\n    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {\n        close(fd);\n        unlink(tempTemplate);\n        throw std::runtime_error("Failed to set file permissions");\n    }\n    \n    // Security: Write with bounds checking\n    ssize_t written = write(fd, content.c_str(), content.length());\n    if (written < 0 || static_cast<size_t>(written) != content.length()) {\n        close(fd);\n        unlink(tempTemplate);\n        throw std::runtime_error("Failed to write content");\n    }\n    \n    // Security: Ensure data is written to disk\n    if (fsync(fd) != 0) {\n        close(fd);\n        unlink(tempTemplate);\n        throw std::runtime_error("Failed to sync file");\n    }\n    \n    close(fd);\n    \n    // Security: Atomic rename\n    if (rename(tempTemplate, targetPath.c_str()) != 0) {\n        unlink(tempTemplate);\n        throw std::runtime_error(std::string("Failed to rename temp file: ") + strerror(errno));\n    }\n#endif\n}\n\nint main() {\n    // Test case 1: Normal write operation\n    try {\n        writeConfig("important_config", "important_config");\n        std::cout << "Test 1 passed: File written successfully" << std::endl;\n    } catch (const std::exception& e) {\n        std::cerr << "Test 1 failed: " << e.what() << std::endl;\n    }\n    \n    // Test case 2: Write with different content\n    try {\n        writeConfig("important_config", "test_content_123");\n        std::cout << "Test 2 passed: File updated successfully" << std::endl;\n    } catch (const std::exception& e) {\n        std::cerr << "Test 2 failed: " << e.what() << std::endl;\n    }\n    \n    // Test case 3: Invalid filename (empty)\n    try {\n        writeConfig("", "content");\n        std::cerr << "Test 3 failed: Should have thrown exception" << std::endl;\n    } catch (const std::invalid_argument& e) {\n        std::cout << "Test 3 passed: Empty filename rejected" << std::endl;\n    } catch (const std::exception& e) {\n        std::cerr << "Test 3 failed with unexpected exception: " << e.what() << std::endl;\n    }\n    \n    // Test case 4: Path traversal attempt\n    try {\n        writeConfig("../../../etc/passwd", "malicious");\n        std::cerr << "Test 4 failed: Path traversal not blocked" << std::endl;\n    } catch (const std::invalid_argument& e) {\n        std::cout << "Test 4 passed: Path traversal blocked" << std::endl;\n    } catch (const std::exception& e) {\n        std::cerr << "Test 4 failed with unexpected exception: " << e.what() << std::endl;\n    }\n    \n    // Test case 5: Empty content\n    try {\n        writeConfig("empty_config", "");\n        std::cout << "Test 5 passed: Empty content written" << std::endl;\n    } catch (const std::exception& e) {\n        std::cerr << "Test 5 failed: " << e.what() << std::endl;
    }
    
    return 0;
}
