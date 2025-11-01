
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cstring>

// Note: This requires a third-party library like libzip or minizip
// For demonstration, showing the secure approach with filesystem checks

#ifdef _WIN32
    // Windows-specific zip handling would require additional libraries
    #include <windows.h>
#else
    #include <zip.h>
#endif

namespace fs = std::filesystem;

bool secureExtractZip(const std::string& zipFilePath, const std::string& destDirectory) {
    try {
        // Create destination directory
        fs::create_directories(destDirectory);
        fs::path destPath = fs::canonical(destDirectory);
        
#ifndef _WIN32
        int err = 0;
        zip* archive = zip_open(zipFilePath.c_str(), ZIP_RDONLY, &err);
        
        if (!archive) {
            std::cerr << "Failed to open zip file" << std::endl;
            return false;
        }
        
        zip_int64_t numEntries = zip_get_num_entries(archive, 0);
        
        for (zip_int64_t i = 0; i < numEntries; i++) {
            const char* name = zip_get_name(archive, i, 0);
            if (!name) continue;
            
            // Security: Validate path
            fs::path memberPath = destPath / name;
            fs::path canonicalMemberPath = fs::weakly_canonical(memberPath);
            
            // Security: Ensure within destination directory
            std::string canonicalStr = canonicalMemberPath.string();
            std::string destStr = destPath.string();
            
            if (canonicalStr.find(destStr) != 0) {
                std::cerr << "Path traversal detected: " << name << std::endl;
                zip_close(archive);
                return false;
            }
            
            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat_index(archive, i, 0, &st);
            
            if (name[strlen(name) - 1] == '/') {
                fs::create_directories(memberPath);
            } else {
                fs::create_directories(memberPath.parent_path());
                
                zip_file* file = zip_fopen_index(archive, i, 0);
                if (!file) continue;
                
                std::ofstream outFile(memberPath, std::ios::binary);
                char buffer[4096];
                zip_int64_t bytesRead;
                
                while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                    outFile.write(buffer, bytesRead);
                }
                
                zip_fclose(file);
                outFile.close();
            }
        }
        
        zip_close(archive);
        return true;
#else
        std::cerr << "Zip extraction not implemented for Windows in this example" << std::endl;
        return false;
#endif
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    // Test case 1: Normal extraction
    if (secureExtractZip("archive.zip", "/tmp/unpack")) {
        std::cout << "Test 1: Successfully extracted archive.zip" << std::endl;
    } else {
        std::cout << "Test 1 failed" << std::endl;
    }
    
    // Test case 2: With different archive name
    if (secureExtractZip("test.zip", "/tmp/unpack")) {
        std::cout << "Test 2: Successfully extracted test.zip" << std::endl;
    } else {
        std::cout << "Test 2 failed" << std::endl;
    }
    
    // Test case 3: Non-existent file
    if (secureExtractZip("nonexistent.zip", "/tmp/unpack")) {
        std::cout << "Test 3: Completed" << std::endl;
    } else {
        std::cout << "Test 3 failed (expected)" << std::endl;
    }
    
    // Test case 4: Different destination
    if (secureExtractZip("archive.zip", "/tmp/unpack2")) {
        std::cout << "Test 4: Successfully extracted to different location" << std::endl;
    } else {
        std::cout << "Test 4 failed" << std::endl;
    }
    
    // Test case 5: Create nested directory structure
    if (secureExtractZip("archive.zip", "/tmp/unpack/nested/path")) {
        std::cout << "Test 5: Successfully extracted to nested path" << std::endl;
    } else {
        std::cout << "Test 5 failed" << std::endl;
    }
    
    return 0;
}
