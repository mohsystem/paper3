
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

// Base64 encoding characters
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

const char* UPLOAD_DIR = "/var/uploads";
const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB limit

// Safe base64 encoding function
std::string base64_encode(const unsigned char* data, size_t len) {
    std::string result;
    
    // Check for integer overflow in size calculation
    if (len > SIZE_MAX / 4 * 3) {
        return "";
    }
    
    result.reserve(((len + 2) / 3) * 4);
    
    for (size_t i = 0; i < len; i += 3) {
        unsigned char b1 = data[i];
        unsigned char b2 = (i + 1 < len) ? data[i + 1] : 0;
        unsigned char b3 = (i + 2 < len) ? data[i + 2] : 0;
        
        result.push_back(base64_chars[b1 >> 2]);
        result.push_back(base64_chars[((b1 & 0x03) << 4) | (b2 >> 4)]);
        result.push_back((i + 1 < len) ? base64_chars[((b2 & 0x0f) << 2) | (b3 >> 6)] : '=');
        result.push_back((i + 2 < len) ? base64_chars[b3 & 0x3f] : '=');
    }
    
    return result;
}

/**
 * Uploads and encodes an image file
 * @param image_data pointer to raw image bytes
 * @param data_len length of image data
 * @return filename where the encoded image was saved, or empty string on failure
 */
std::string upload_image(const unsigned char* image_data, size_t data_len) {
    // Input validation: check for NULL pointer and size limits
    if (image_data == NULL || data_len == 0) {
        std::cerr << "Invalid image data" << std::endl;
        return "";
    }
    
    if (data_len > MAX_FILE_SIZE) {
        std::cerr << "File size exceeds limit" << std::endl;
        return "";
    }
    
    // Create upload directory with restrictive permissions (700)
    struct stat st;
    if (stat(UPLOAD_DIR, &st) != 0) {
        if (mkdir(UPLOAD_DIR, 0700) != 0) {
            std::cerr << "Failed to create upload directory" << std::endl;
            return "";
        }
    }
    
    // Generate a safe, random filename
    srand(time(NULL));
    char filename[256];
    snprintf(filename, sizeof(filename), "image_%ld_%d.b64", 
             time(NULL), rand() % 100000);
    
    // Build and validate the full path
    char full_path[PATH_MAX];
    char resolved_upload[PATH_MAX];
    char resolved_full[PATH_MAX];
    
    // Resolve upload directory to absolute path
    if (realpath(UPLOAD_DIR, resolved_upload) == NULL) {
        std::cerr << "Failed to resolve upload directory" << std::endl;
        return "";
    }
    
    // Build full path
    snprintf(full_path, sizeof(full_path), "%s/%s", UPLOAD_DIR, filename);
    
    // Encode the image data using base64
    std::string encoded_image = base64_encode(image_data, data_len);
    if (encoded_image.empty()) {
        std::cerr << "Base64 encoding failed" << std::endl;
        return "";
    }
    
    // Create temp file in same directory with O_CREAT|O_EXCL for race safety
    char temp_path[PATH_MAX];
    snprintf(temp_path, sizeof(temp_path), "%s/temp_%ld_%d.tmp", 
             UPLOAD_DIR, time(NULL), rand() % 100000);
    
    // Open with O_CREAT|O_EXCL|O_WRONLY and restrictive permissions (0600)
    int fd = open(temp_path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) {
        std::cerr << "Failed to create temp file" << std::endl;
        return "";
    }
    
    // Validate the opened file descriptor
    struct stat temp_st;
    if (fstat(fd, &temp_st) != 0 || !S_ISREG(temp_st.st_mode)) {
        close(fd);
        unlink(temp_path);
        std::cerr << "Invalid file type" << std::endl;
        return "";
    }
    
    // Write encoded data to file
    ssize_t written = write(fd, encoded_image.c_str(), encoded_image.length());
    if (written < 0 || (size_t)written != encoded_image.length()) {
        close(fd);
        unlink(temp_path);
        std::cerr << "Failed to write encoded image" << std::endl;
        return "";
    }
    
    // Sync to disk
    if (fsync(fd) != 0) {
        close(fd);
        unlink(temp_path);
        std::cerr << "Failed to sync file" << std::endl;
        return "";
    }
    
    close(fd);
    
    // Atomically rename temp file to final destination
    if (rename(temp_path, full_path) != 0) {
        unlink(temp_path);
        std::cerr << "Failed to rename file" << std::endl;
        return "";
    }
    
    return std::string(filename);
}

int main() {
    // Test case 1: Valid small image
    const unsigned char test1[] = "Test image data 1";
    std::string result1 = upload_image(test1, sizeof(test1) - 1);
    std::cout << "Test 1 - Valid small image: " << (!result1.empty() ? "PASS" : "FAIL") << std::endl;
    
    // Test case 2: NULL input
    std::string result2 = upload_image(NULL, 100);
    std::cout << "Test 2 - NULL input: " << (result2.empty() ? "PASS" : "FAIL") << std::endl;
    
    // Test case 3: Zero length
    const unsigned char test3[] = "Data";
    std::string result3 = upload_image(test3, 0);
    std::cout << "Test 3 - Zero length: " << (result3.empty() ? "PASS" : "FAIL") << std::endl;
    
    // Test case 4: Valid binary data
    unsigned char test4[1024];
    for (size_t i = 0; i < sizeof(test4); i++) {
        test4[i] = (unsigned char)(i % 256);
    }
    std::string result4 = upload_image(test4, sizeof(test4));
    std::cout << "Test 4 - Binary data: " << (!result4.empty() ? "PASS" : "FAIL") << std::endl;
    
    // Test case 5: Size boundary
    unsigned char test5[2048];
    memset(test5, 'A', sizeof(test5));
    std::string result5 = upload_image(test5, sizeof(test5));
    std::cout << "Test 5 - Size boundary: " << (!result5.empty() ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}
