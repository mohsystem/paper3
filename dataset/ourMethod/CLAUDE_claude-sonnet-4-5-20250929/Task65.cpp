
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>

// Maximum file size to prevent excessive memory usage (10 MB)
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
// Maximum filename length for validation
const size_t MAX_FILENAME_LENGTH = 4096;

// Validates filename format and checks for path traversal attempts
bool validateFilename(const std::string& filename) {
    // Check filename length to prevent buffer issues
    if (filename.empty() || filename.length() > MAX_FILENAME_LENGTH) {
        return false;
    }
    
    // Reject null bytes in filename
    if (filename.find('\\0') != std::string::npos) {
        return false;
    }
    
    // Reject path traversal sequences
    if (filename.find("..") != std::string::npos) {
        return false;
    }
    
    // Reject absolute paths (security: restrict to current directory)
    if (!filename.empty() && filename[0] == '/') {
        return false;
    }
    
    // Reject Windows absolute paths
    if (filename.length() >= 2 && filename[1] == ':') {
        return false;
    }
    
    // Reject special characters that could be dangerous
    const std::string dangerous_chars = "\\n\\r\\t|&;$<>`!";
    if (filename.find_first_of(dangerous_chars) != std::string::npos) {
        return false;
    }
    
    return true;
}

// Checks if path is a regular file and accessible
bool isRegularFile(const std::string& filepath) {
    struct stat path_stat;
    // Check if stat succeeds
    if (stat(filepath.c_str(), &path_stat) != 0) {
        return false;
    }
    
    // Reject directories - require regular file
    if (S_ISDIR(path_stat.st_mode)) {
        return false;
    }
    
    // Reject symlinks to prevent TOCTOU and traversal
    if (S_ISLNK(path_stat.st_mode)) {
        return false;
    }
    
    // Must be a regular file
    if (!S_ISREG(path_stat.st_mode)) {
        return false;
    }
    
    // Check read access
    if (access(filepath.c_str(), R_OK) != 0) {
        return false;
    }
    
    return true;
}

// Retrieves file data with security checks
std::vector<unsigned char> getFileData(const std::string& filename, std::string& errorMsg) {
    std::vector<unsigned char> data;
    errorMsg.clear();
    
    // Validate filename format
    if (!validateFilename(filename)) {
        errorMsg = "Invalid filename format";
        return data;
    }
    
    // Check if file exists and is a regular file
    if (!isRegularFile(filename)) {
        errorMsg = "File does not exist, is not accessible, or is not a regular file";
        return data;
    }
    
    // Get file size before opening
    struct stat file_stat;
    if (stat(filename.c_str(), &file_stat) != 0) {
        errorMsg = "Cannot stat file";
        return data;
    }
    
    // Validate file size to prevent excessive memory allocation
    if (file_stat.st_size < 0 || static_cast<unsigned long long>(file_stat.st_size) > MAX_FILE_SIZE) {
        errorMsg = "File size invalid or exceeds maximum allowed size";
        return data;
    }
    
    // Open file in binary mode with RAII
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open() || !file.good()) {
        errorMsg = "Cannot open file for reading";
        return data;
    }
    
    // Reserve space based on file size
    size_t fileSize = static_cast<size_t>(file_stat.st_size);
    try {
        data.reserve(fileSize);
    } catch (const std::bad_alloc&) {
        errorMsg = "Memory allocation failed";
        return data;
    }
    
    // Read file in chunks with bounds checking
    const size_t CHUNK_SIZE = 8192;
    std::vector<unsigned char> buffer(CHUNK_SIZE);
    size_t totalRead = 0;
    
    while (file.good() && totalRead < MAX_FILE_SIZE) {
        // Read chunk with bounds check
        file.read(reinterpret_cast<char*>(buffer.data()), CHUNK_SIZE);
        std::streamsize bytesRead = file.gcount();
        
        if (bytesRead < 0) {
            errorMsg = "Read error occurred";
            data.clear();
            return data;
        }
        
        // Check for overflow before adding
        if (totalRead + static_cast<size_t>(bytesRead) > MAX_FILE_SIZE) {
            errorMsg = "File size exceeds maximum during read";
            data.clear();
            return data;
        }
        
        // Append read bytes with bounds safety
        try {
            data.insert(data.end(), buffer.begin(), buffer.begin() + bytesRead);
        } catch (const std::bad_alloc&) {
            errorMsg = "Memory allocation failed during read";
            data.clear();
            return data;
        }
        
        totalRead += static_cast<size_t>(bytesRead);
        
        if (file.eof()) {
            break;
        }
    }
    
    // Check for read errors
    if (file.bad()) {
        errorMsg = "Error reading file";
        data.clear();
        return data;
    }
    
    file.close();
    return data;
}

int main(int argc, char* argv[]) {
    // Validate command line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "program") << " <filename>" << std::endl;
        return 1;
    }
    
    // Validate argv[1] is not NULL (defensive check)
    if (argv[1] == nullptr) {
        std::cerr << "Error: NULL filename argument" << std::endl;
        return 1;
    }
    
    std::string filename(argv[1]);
    std::string errorMsg;
    
    // Test cases
    std::vector<std::string> testFiles = {
        "test1.txt",
        "test2.dat",
        "../etc/passwd",  // Should be rejected
        "/etc/hosts",     // Should be rejected
        "nonexistent.txt"
    };
    
    std::cout << "=== Running Test Cases ===" << std::endl;
    for (const auto& testFile : testFiles) {
        std::cout << "\\nTesting file: " << testFile << std::endl;
        std::vector<unsigned char> fileData = getFileData(testFile, errorMsg);
        
        if (fileData.empty() && !errorMsg.empty()) {
            std::cout << "Error: " << errorMsg << std::endl;
        } else if (fileData.empty()) {
            std::cout << "File is empty or could not be read" << std::endl;
        } else {
            std::cout << "Success! Read " << fileData.size() << " bytes" << std::endl;
            // Print first 50 bytes or less
            std::cout << "First bytes (hex): ";
            size_t printLimit = fileData.size() < 50 ? fileData.size() : 50;
            for (size_t i = 0; i < printLimit; ++i) {
                printf("%02x ", fileData[i]);
            }
            std::cout << std::endl;
        }
    }
    
    // Process the actual command line argument
    std::cout << "\\n=== Processing Command Line Argument ===" << std::endl;
    std::cout << "Reading file: " << filename << std::endl;
    std::vector<unsigned char> result = getFileData(filename, errorMsg);
    
    if (result.empty() && !errorMsg.empty()) {
        std::cerr << "Error: " << errorMsg << std::endl;
        return 1;
    } else if (result.empty()) {
        std::cout << "File is empty" << std::endl;
        return 0;
    } else {
        std::cout << "Successfully read " << result.size() << " bytes from file" << std::endl;
        return 0;
    }
}
