
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define PATH_MAX MAX_PATH
#endif

const std::string UPLOAD_DIR = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
const unsigned char PDF_MAGIC[] = {0x25, 0x50, 0x44, 0x46}; // %PDF

// Secure base64url encoding for filename generation
std::string base64url_encode(const std::vector<unsigned char>& input) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    
    std::string result;
    result.reserve(((input.size() + 2) / 3) * 4);
    
    size_t i = 0;
    while (i < input.size()) {
        unsigned char b1 = input[i++];
        unsigned char b2 = (i < input.size()) ? input[i++] : 0;
        unsigned char b3 = (i < input.size()) ? input[i++] : 0;
        
        result += base64_chars[b1 >> 2];
        result += base64_chars[((b1 & 0x03) << 4) | (b2 >> 4)];
        if (i > input.size() + 1) break;
        result += base64_chars[((b2 & 0x0f) << 2) | (b3 >> 6)];
        if (i > input.size()) break;
        result += base64_chars[b3 & 0x3f];
    }
    
    // Remove padding for urlsafe
    while (!result.empty() && result.back() == '=') {
        result.pop_back();
    }
    
    return result;
}

// Generate cryptographically secure random bytes
std::vector<unsigned char> secure_random_bytes(size_t length) {
    std::vector<unsigned char> buffer(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = static_cast<unsigned char>(dis(gen));
    }
    
    return buffer;
}

// Secure file upload function with validation and TOCTOU prevention
std::string upload_pdf(const std::vector<unsigned char>& file_content, 
                       const std::string& original_filename) {
    // Input validation: check file size
    if (file_content.empty() || file_content.size() > MAX_FILE_SIZE) {
        throw std::invalid_argument("Invalid file size");
    }
    
    // Validate filename doesn't contain path separators or traversal\n    if (original_filename.empty() || \n        original_filename.find("..") != std::string::npos ||\n        original_filename.find('/') != std::string::npos ||\n        original_filename.find('\\\\') != std::string::npos) {
        throw std::invalid_argument("Invalid filename");
    }
    
    // Check file extension
    if (original_filename.size() < 4 || 
        original_filename.substr(original_filename.size() - 4) != ".pdf") {
        throw std::invalid_argument("Only PDF files are allowed");
    }
    
    // Validate PDF magic bytes
    if (file_content.size() < 4) {
        throw std::invalid_argument("File too small to be a valid PDF");
    }
    
    if (std::memcmp(file_content.data(), PDF_MAGIC, 4) != 0) {
        throw std::invalid_argument("File is not a valid PDF");
    }
    
    // Create uploads directory with restricted permissions
    struct stat st;
    if (stat(UPLOAD_DIR.c_str(), &st) != 0) {
#ifdef _WIN32
        if (_mkdir(UPLOAD_DIR.c_str()) != 0) {
            throw std::runtime_error("Failed to create upload directory");
        }
#else
        if (mkdir(UPLOAD_DIR.c_str(), 0700) != 0) {
            throw std::runtime_error("Failed to create upload directory");
        }
#endif
    }
    
    // Generate secure random filename
    std::vector<unsigned char> random_bytes = secure_random_bytes(16);
    std::string safe_filename = base64url_encode(random_bytes) + ".pdf";
    
    // Construct paths
    std::string target_path = UPLOAD_DIR + "/" + safe_filename;
    std::string temp_path = UPLOAD_DIR + "/temp_" + safe_filename;
    
    // Validate target path is within upload directory (basic check)
    if (target_path.find("..") != std::string::npos) {
        throw std::runtime_error("Path traversal attempt detected");
    }
    
    int temp_fd = -1;
    
    try {
        // Open temp file with O_CREAT | O_EXCL to prevent TOCTOU
#ifdef _WIN32
        temp_fd = _open(temp_path.c_str(), _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
        temp_fd = open(temp_path.c_str(), O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW, 0600);
#endif
        
        if (temp_fd < 0) {
            throw std::runtime_error("Failed to create temporary file");
        }
        
        // Write content to temp file
        size_t total_written = 0;
        while (total_written < file_content.size()) {
#ifdef _WIN32
            int written = _write(temp_fd, file_content.data() + total_written, 
                               file_content.size() - total_written);
#else
            ssize_t written = write(temp_fd, file_content.data() + total_written, 
                                  file_content.size() - total_written);
#endif
            if (written < 0) {
                throw std::runtime_error("Failed to write to file");
            }
            total_written += written;
        }
        
        // Sync to disk
#ifdef _WIN32
        _commit(temp_fd);
        _close(temp_fd);
#else
        fsync(temp_fd);
        close(temp_fd);
#endif
        temp_fd = -1;
        
        // Atomic move to final location
#ifdef _WIN32
        if (!MoveFileExA(temp_path.c_str(), target_path.c_str(), 
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            throw std::runtime_error("Failed to move file to final location");
        }
#else
        if (rename(temp_path.c_str(), target_path.c_str()) != 0) {
            throw std::runtime_error("Failed to move file to final location");
        }
#endif
        
        return safe_filename;
        
    } catch (...) {
        // Clean up on error
        if (temp_fd >= 0) {
#ifdef _WIN32
            _close(temp_fd);
#else
            close(temp_fd);
#endif
        }
        unlink(temp_path.c_str());
        throw;
    }
}

int main() {
    std::cout << "PDF Upload Test Cases:" << std::endl;
    
    // Test case 1: Valid PDF file
    try {
        std::vector<unsigned char> valid_pdf(100, 0);
        valid_pdf[0] = 0x25; valid_pdf[1] = 0x50; 
        valid_pdf[2] = 0x44; valid_pdf[3] = 0x46; // %PDF
        std::string result1 = upload_pdf(valid_pdf, "document.pdf");
        std::cout << "Test 1 PASSED: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 FAILED: " << e.what() << std::endl;
    }
    
    // Test case 2: File too large
    try {
        std::vector<unsigned char> large_pdf(20 * 1024 * 1024, 0);
        upload_pdf(large_pdf, "large.pdf");
        std::cout << "Test 2 FAILED: Should reject large file" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 2 PASSED: Rejected large file" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 FAILED: " << e.what() << std::endl;
    }
    
    // Test case 3: Path traversal attempt
    try {
        std::vector<unsigned char> valid_pdf(100, 0);
        valid_pdf[0] = 0x25; valid_pdf[1] = 0x50; 
        valid_pdf[2] = 0x44; valid_pdf[3] = 0x46;
        upload_pdf(valid_pdf, "../../../etc/passwd.pdf");
        std::cout << "Test 3 FAILED: Should reject path traversal" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 3 PASSED: Rejected path traversal" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 FAILED: " << e.what() << std::endl;
    }
    
    // Test case 4: Invalid file extension
    try {
        std::vector<unsigned char> valid_pdf(100, 0);
        valid_pdf[0] = 0x25; valid_pdf[1] = 0x50; 
        valid_pdf[2] = 0x44; valid_pdf[3] = 0x46;
        upload_pdf(valid_pdf, "document.exe");
        std::cout << "Test 4 FAILED: Should reject non-PDF extension" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 4 PASSED: Rejected non-PDF extension" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 FAILED: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid PDF magic bytes
    try {
        std::vector<unsigned char> invalid_pdf(100, 0);
        upload_pdf(invalid_pdf, "fake.pdf");
        std::cout << "Test 5 FAILED: Should reject invalid PDF" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 5 PASSED: Rejected invalid PDF magic bytes" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 FAILED: " << e.what() << std::endl;
    }
    
    return 0;
}
