
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cstring>
#include <cstdlib>
#include <regex>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// Security: Define maximum file size to prevent DoS (10MB)
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
// Security: Define allowed file extensions whitelist
const std::set<std::string> ALLOWED_EXTENSIONS = {".txt", ".pdf", ".jpg", ".png"};
// Security: Define upload directory
const std::string UPLOAD_DIR = "uploads";
// Security: Maximum filename length to prevent buffer issues
const size_t MAX_FILENAME_LENGTH = 255;

class SecureFileUploader {
public:
    SecureFileUploader() {
        // Security: Create upload directory with restrictive permissions
        createUploadDirectory();
    }

    bool isValidFilename(const std::string& filename) const {
        // Security: Validate filename length
        if (filename.empty() || filename.length() > MAX_FILENAME_LENGTH) {
            return false;
        }

        // Security: Extract basename to prevent path traversal
        std::string basename = getBasename(filename);

        // Security: Validate filename contains only safe characters
        std::regex safe_pattern("^[a-zA-Z0-9_.-]+$");
        if (!std::regex_match(basename, safe_pattern)) {
            return false;
        }

        // Security: Check file extension against whitelist
        std::string extension = getExtension(basename);
        if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {
            return false;
        }

        return true;
    }

    std::string saveFile(const std::string& filename, const std::vector<uint8_t>& content) {
        try {
            // Security: Validate content size
            if (content.size() > MAX_FILE_SIZE) {
                return "Error: File too large";
            }

            // Security: Validate filename
            if (!isValidFilename(filename)) {
                return "Error: Invalid filename";
            }

            // Security: Generate random unique prefix
            unsigned char random_bytes[16] = {0};
            if (RAND_bytes(random_bytes, sizeof(random_bytes)) != 1) {
                return "Error: Failed to generate random data";
            }

            unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
            SHA256(random_bytes, sizeof(random_bytes), hash);

            char hex_prefix[33] = {0};
            for (int i = 0; i < 16; i++) {
                snprintf(hex_prefix + (i * 2), 3, "%02x", hash[i]);
            }
            hex_prefix[16] = '\\0';

            std::string safe_filename = std::string(hex_prefix) + "_" + getBasename(filename);

            // Security: Construct path safely within upload directory
            std::string target_path = UPLOAD_DIR + "/" + safe_filename;

            // Security: Validate resolved path is within upload directory
            if (target_path.find("..") != std::string::npos) {
                return "Error: Invalid file path";
            }

            // Security: Create temp file first for atomic write
            std::string temp_filename = UPLOAD_DIR + "/upload_";
            for (int i = 0; i < 8; i++) {
                unsigned char rand_byte;
                RAND_bytes(&rand_byte, 1);
                char hex[3];
                snprintf(hex, sizeof(hex), "%02x", rand_byte);
                temp_filename += hex;
            }
            temp_filename += ".tmp";

            // Security: Open temp file with O_CREAT | O_EXCL to prevent races
            int fd = open(temp_filename.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
            if (fd < 0) {
                return "Error: Failed to create temp file";
            }

            // Security: Write content to temp file
            ssize_t written = write(fd, content.data(), content.size());
            if (written < 0 || static_cast<size_t>(written) != content.size()) {
                close(fd);
                unlink(temp_filename.c_str());
                return "Error: Failed to write file";
            }

            // Security: Sync to disk
            fsync(fd);
            close(fd);

            // Security: Atomic rename to final location
            if (rename(temp_filename.c_str(), target_path.c_str()) != 0) {
                unlink(temp_filename.c_str());
                return "Error: Failed to move file";
            }

            return "File uploaded successfully";

        } catch (const std::exception& e) {
            return "Error: File save failed";
        }
    }

private:
    void createUploadDirectory() {
        // Security: Create directory with restrictive permissions (0700)
#ifdef _WIN32
        _mkdir(UPLOAD_DIR.c_str());
#else
        mkdir(UPLOAD_DIR.c_str(), 0700);
#endif
    }

    std::string getBasename(const std::string& path) const {
        // Security: Extract basename to prevent path traversal
        size_t pos = path.find_last_of("/\\\\");\n        if (pos != std::string::npos) {\n            return path.substr(pos + 1);\n        }\n        return path;\n    }\n\n    std::string getExtension(const std::string& filename) const {\n        size_t pos = filename.find_last_of('.');\n        if (pos != std::string::npos && pos > 0) {\n            std::string ext = filename.substr(pos);\n            // Convert to lowercase\n            for (char& c : ext) {\n                c = std::tolower(c);\n            }\n            return ext;\n        }\n        return "";\n    }\n};\n\nvoid runTests() {\n    std::cout << "\
=== Running Test Cases ===" << std::endl;\n    \n    SecureFileUploader uploader;\n\n    // Test 1: Valid filename\n    std::cout << "Test 1: Valid filename 'test.txt' - Expected: Pass" << std::endl;\n    std::cout << "Result: " << (uploader.isValidFilename("test.txt") ? "PASS" : "FAIL") << std::endl;\n\n    // Test 2: Path traversal attempt\n    std::cout << "\
Test 2: Path traversal '../etc/passwd' - Expected: Fail" << std::endl;\n    std::cout << "Result: " << (!uploader.isValidFilename("../etc/passwd") ? "PASS" : "FAIL") << std::endl;\n\n    // Test 3: Invalid extension\n    std::cout << "\
Test 3: Invalid extension 'file.exe' - Expected: Fail" << std::endl;\n    std::cout << "Result: " << (!uploader.isValidFilename("file.exe") ? "PASS" : "FAIL") << std::endl;\n\n    // Test 4: Special characters\n    std::cout << "\
Test 4: Special characters 'file<>.txt' - Expected: Fail" << std::endl;\n    std::cout << "Result: " << (!uploader.isValidFilename("file<>.txt") ? "PASS" : "FAIL") << std::endl;\n\n    // Test 5: Maximum length\n    std::cout << "\
Test 5: Filename exceeds max length - Expected: Fail" << std::endl;\n    std::string long_name(256, 'a');\n    long_name += ".txt";\n    std::cout << "Result: " << (!uploader.isValidFilename(long_name) ? "PASS" : "FAIL") << std::endl;\n}\n\nint main() {\n    // Security: Initialize OpenSSL random number generator\n    if (RAND_status() != 1) {\n        std::cerr << "Failed to initialize random number generator" << std::endl;\n        return 1;\n    }\n\n    SecureFileUploader uploader;\n    \n    std::cout << "Secure File Upload Server Initialized" << std::endl;\n    std::cout << "Upload directory: " << UPLOAD_DIR << std::endl;\n    std::cout << "Max file size: " << MAX_FILE_SIZE << " bytes" << std::endl;

    // Run test cases
    runTests();

    return 0;
}
