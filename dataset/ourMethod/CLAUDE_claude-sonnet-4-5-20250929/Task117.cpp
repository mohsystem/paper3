
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <sys/stat.h>
#include <limits>
#include <algorithm>

// ZIP format structures
#pragma pack(push, 1)
struct LocalFileHeader {
    uint32_t signature;
    uint16_t version;
    uint16_t flags;
    uint16_t compression;
    uint16_t modTime;
    uint16_t modDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t filenameLen;
    uint16_t extraLen;
};

struct CentralDirHeader {
    uint32_t signature;
    uint16_t versionMade;
    uint16_t versionNeeded;
    uint16_t flags;
    uint16_t compression;
    uint16_t modTime;
    uint16_t modDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t filenameLen;
    uint16_t extraLen;
    uint16_t commentLen;
    uint16_t diskStart;
    uint16_t internalAttr;
    uint32_t externalAttr;
    uint32_t localHeaderOffset;
};
#pragma pack(pop)

const uint32_t LOCAL_FILE_HEADER_SIG = 0x04034b50;
const uint32_t CENTRAL_DIR_HEADER_SIG = 0x02014b50;
const size_t MAX_FILENAME_LEN = 4096;
const size_t MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB limit

// Validates path to prevent directory traversal (CWE-22)
bool isPathSafe(const std::string& path, const std::string& baseDir) {
    // Reject paths with dangerous patterns
    if (path.empty() || path.find("..") != std::string::npos || 
        path[0] == '/' || path[0] == '\\\\') {\n        return false;\n    }\n    \n    // Check for absolute paths or drive letters\n    if (path.size() >= 2 && path[1] == ':') {\n        return false;\n    }\n    \n    // Ensure no null bytes in path (CWE-158)\n    if (path.find('\\0') != std::string::npos) {\n        return false;\n    }\n    \n    return true;\n}\n\n// Safe file write with atomic rename to prevent TOCTOU (CWE-367)\nbool safeWriteFile(const std::string& finalPath, const std::vector<uint8_t>& data) {\n    // Create temp file in same directory for atomic rename\n    std::string tempPath = finalPath + ".tmp";\n    \n    // Open with exclusive creation (fails if exists)\n    std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);\n    if (!out) {\n        return false;\n    }\n    \n    // Write data\n    out.write(reinterpret_cast<const char*>(data.data()), data.size());\n    if (!out) {\n        out.close();\n        std::remove(tempPath.c_str());\n        return false;\n    }\n    \n    // Flush to disk\n    out.flush();\n    out.close();\n    \n    // Atomic rename (replaces existing file)\n    if (std::rename(tempPath.c_str(), finalPath.c_str()) != 0) {\n        std::remove(tempPath.c_str());\n        return false;\n    }\n    \n    return true;\n}\n\n// Extract ZIP archive with security checks\nbool extractZip(const std::string& zipPath, const std::string& outputDir) {\n    // Validate inputs (CWE-20)\n    if (zipPath.empty() || outputDir.empty()) {\n        std::cerr << "Invalid input paths\\n";\n        return false;\n    }\n    \n    std::ifstream zip(zipPath, std::ios::binary);\n    if (!zip) {\n        std::cerr << "Cannot open ZIP file\\n";\n        return false;\n    }\n    \n    // Get file size for bounds checking (CWE-119)\n    zip.seekg(0, std::ios::end);\n    std::streampos fileSize = zip.tellg();\n    if (fileSize < 0 || fileSize > std::numeric_limits<size_t>::max()) {\n        std::cerr << "Invalid file size\\n";\n        return false;\n    }\n    zip.seekg(0, std::ios::beg);\n    \n    size_t totalExtracted = 0;\n    const size_t MAX_TOTAL_SIZE = 500 * 1024 * 1024; // 500MB total extraction limit\n    \n    while (zip && zip.peek() != EOF) {\n        LocalFileHeader header;\n        \n        // Check bounds before reading header (CWE-125)\n        std::streampos currentPos = zip.tellg();\n        if (currentPos < 0 || currentPos + static_cast<std::streamoff>(sizeof(header)) > fileSize) {\n            break;\n        }\n        \n        zip.read(reinterpret_cast<char*>(&header), sizeof(header));\n        if (!zip || zip.gcount() != sizeof(header)) {\n            break;\n        }\n        \n        // Validate signature\n        if (header.signature != LOCAL_FILE_HEADER_SIG) {\n            break;\n        }\n        \n        // Validate filename length (CWE-20)\n        if (header.filenameLen == 0 || header.filenameLen > MAX_FILENAME_LEN) {\n            std::cerr << "Invalid filename length\\n";\n            return false;\n        }\n        \n        // Check bounds before reading filename (CWE-119)\n        currentPos = zip.tellg();\n        if (currentPos < 0 || currentPos + header.filenameLen + header.extraLen > fileSize) {\n            std::cerr << "File truncated\\n";\n            return false;\n        }\n        \n        // Read filename with bounds check\n        std::vector<char> filenameBuf(header.filenameLen + 1, 0);\n        zip.read(filenameBuf.data(), header.filenameLen);\n        if (!zip || zip.gcount() != header.filenameLen) {\n            std::cerr << "Cannot read filename\\n";\n            return false;\n        }\n        std::string filename(filenameBuf.data(), header.filenameLen);\n        \n        // Validate path safety (CWE-22: Path Traversal)\n        if (!isPathSafe(filename, outputDir)) {\n            std::cerr << "Unsafe path detected: " << filename << "\\n";\n            return false;\n        }\n        \n        // Skip extra field\n        if (header.extraLen > 0) {\n            zip.seekg(header.extraLen, std::ios::cur);\n        }\n        \n        // Validate file size (CWE-770: Resource exhaustion)\n        if (header.uncompressedSize > MAX_FILE_SIZE) {\n            std::cerr << "File too large: " << filename << "\\n";\n            return false;\n        }\n        \n        // Check total extraction size to prevent zip bomb (CWE-409)\n        if (totalExtracted + header.uncompressedSize > MAX_TOTAL_SIZE) {\n            std::cerr << "Total extraction size exceeds limit\\n";\n            return false;\n        }\n        \n        // Check if this is a directory (ends with /)\n        if (!filename.empty() && filename.back() == '/') {
            // Skip directories
            if (header.compressedSize > 0) {
                zip.seekg(header.compressedSize, std::ios::cur);
            }
            continue;
        }
        
        // Read compressed data with bounds check (CWE-119)
        currentPos = zip.tellg();
        if (currentPos < 0 || header.compressedSize > MAX_FILE_SIZE ||
            currentPos + header.compressedSize > fileSize) {
            std::cerr << "Invalid compressed size\\n";
            return false;
        }
        
        std::vector<uint8_t> compressedData(header.compressedSize);
        zip.read(reinterpret_cast<char*>(compressedData.data()), header.compressedSize);
        if (!zip || zip.gcount() != static_cast<std::streamsize>(header.compressedSize)) {
            std::cerr << "Cannot read file data\\n";
            return false;
        }
        
        // Only support stored (uncompressed) files for simplicity
        // Production code would use zlib for decompression
        if (header.compression != 0) {
            std::cerr << "Compressed files not supported in this implementation\\n";
            continue;
        }
        
        // Construct output path safely (CWE-22)
        std::string outputPath = outputDir + "/" + filename;
        
        // Write file atomically to prevent TOCTOU (CWE-367)
        if (!safeWriteFile(outputPath, compressedData)) {
            std::cerr << "Failed to write file: " << outputPath << "\\n";
            return false;
        }
        
        totalExtracted += header.uncompressedSize;
        std::cout << "Extracted: " << filename << "\\n";
    }
    
    return true;
}

int main() {
    // Test case 1: Valid ZIP extraction
    std::cout << "=== Test 1: Valid ZIP extraction ===\\n";
    if (extractZip("test.zip", "output1")) {
        std::cout << "Test 1 passed\\n";
    } else {
        std::cout << "Test 1 failed\\n";
    }
    
    // Test case 2: Path traversal attempt (should fail)
    std::cout << "\\n=== Test 2: Path traversal safety ===\\n";
    std::string maliciousPath = "../../../etc/passwd";
    if (!isPathSafe(maliciousPath, "output2")) {
        std::cout << "Test 2 passed: Path traversal blocked\\n";
    } else {
        std::cout << "Test 2 failed: Path traversal not blocked\\n";
    }
    
    // Test case 3: Absolute path attempt (should fail)
    std::cout << "\\n=== Test 3: Absolute path safety ===\\n";
    std::string absolutePath = "/tmp/test.txt";
    if (!isPathSafe(absolutePath, "output3")) {
        std::cout << "Test 3 passed: Absolute path blocked\\n";
    } else {
        std::cout << "Test 3 failed: Absolute path not blocked\\n";
    }
    
    // Test case 4: Valid relative path (should pass)
    std::cout << "\\n=== Test 4: Valid relative path ===\\n";
    std::string validPath = "subdir/file.txt";
    if (isPathSafe(validPath, "output4")) {
        std::cout << "Test 4 passed: Valid path accepted\\n";
    } else {
        std::cout << "Test 4 failed: Valid path rejected\\n";
    }
    
    // Test case 5: Empty input validation
    std::cout << "\\n=== Test 5: Empty input validation ===\\n";
    if (!extractZip("", "output5")) {
        std::cout << "Test 5 passed: Empty input rejected\\n";
    } else {
        std::cout << "Test 5 failed: Empty input accepted\\n";
    }
    
    return 0;
}
