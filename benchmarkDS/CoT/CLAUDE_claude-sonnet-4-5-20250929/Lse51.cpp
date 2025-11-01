
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <cstring>
#include <stdexcept>

namespace fs = std::filesystem;

class Lse51 {
private:
    static constexpr const char* EXTRACT_DIR = "/tmp/unpack";
    
    static bool isPathSafe(const fs::path& basePath, const fs::path& targetPath) {
        auto canonicalBase = fs::canonical(basePath);
        auto normalizedTarget = fs::weakly_canonical(targetPath);
        
        // Check if target is within base directory
        auto mismatch = std::mismatch(canonicalBase.begin(), canonicalBase.end(),
                                     normalizedTarget.begin(), normalizedTarget.end());
        return mismatch.first == canonicalBase.end();
    }
    
public:
    static void extractTarArchive(const std::string& tarFilePath) {
        fs::path extractPath(EXTRACT_DIR);
        fs::create_directories(extractPath);
        
        struct archive* a = archive_read_new();
        struct archive_entry* entry;
        
        // Enable all formats and filters
        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);
        
        if (archive_read_open_filename(a, tarFilePath.c_str(), 10240) != ARCHIVE_OK) {
            archive_read_free(a);
            throw std::runtime_error("Failed to open archive: " + std::string(archive_error_string(a)));
        }
        
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            const char* entryName = archive_entry_pathname(entry);
            
            // Security: Check for path traversal
            if (std::string(entryName).find("..") != std::string::npos ||
                entryName[0] == '/') {
                archive_read_free(a);
                throw std::runtime_error("Path traversal attempt detected");
            }
            
            fs::path targetPath = extractPath / entryName;
            
            // Security: Validate path is within extract directory
            if (!isPathSafe(extractPath, targetPath)) {
                archive_read_free(a);
                throw std::runtime_error("Invalid path detected: " + std::string(entryName));
            }
            
            // Update entry path to absolute target path
            archive_entry_set_pathname(entry, targetPath.string().c_str());
            
            // Extract entry
            if (archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM) != ARCHIVE_OK) {
                std::cerr << "Warning: Failed to extract " << entryName << ": " 
                         << archive_error_string(a) << std::endl;
            }
        }
        
        archive_read_close(a);
        archive_read_free(a);
    }
    
    static void writeFileExample(const std::string& filePath, const std::string& content) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing");
        }
        file << content;
    }
    
    static std::string readFileExample(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for reading");
        }
        return std::string((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    }
};

int main() {
    const std::string testCases[] = {
        "/tmp/test1.tar",
        "/tmp/test2.tar.gz",
        "/tmp/test3.tar.bz2",
        "/tmp/test4.tgz",
        "/tmp/test5.tar"
    };
    
    for (const auto& testFile : testCases) {
        try {
            std::cout << "Extracting: " << testFile << std::endl;
            Lse51::extractTarArchive(testFile);
            std::cout << "Successfully extracted to /tmp/unpack" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error extracting " << testFile << ": " << e.what() << std::endl;
        }
    }
    
    // Demonstrate file operations
    try {
        std::string testFilePath = std::string(EXTRACT_DIR) + "/test.txt";
        Lse51::writeFileExample(testFilePath, "Test content\\n");
        std::string content = Lse51::readFileExample(testFilePath);
        std::cout << "File content: " << content << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "File operation error: " << e.what() << std::endl;
    }
    
    return 0;
}
