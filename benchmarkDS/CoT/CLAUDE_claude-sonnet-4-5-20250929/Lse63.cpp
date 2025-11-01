
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/stat.h>

const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

struct ChecksumResult {
    size_t bytesRead;
    unsigned long checksum;
    
    ChecksumResult(size_t bytes = 0, unsigned long sum = 0) 
        : bytesRead(bytes), checksum(sum) {}
};

bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

size_t getFileSize(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        return 0;
    }
    return buffer.st_size;
}

ChecksumResult processFile(const std::string& inputPath, const std::string& outputPath) {
    if (inputPath.empty() || outputPath.empty()) {
        std::cerr << "Error: File paths cannot be empty" << std::endl;
        return ChecksumResult(0, 0);
    }
    
    // Validate input file
    if (!fileExists(inputPath)) {
        std::cerr << "Error: Input file does not exist" << std::endl;
        return ChecksumResult(0, 0);
    }
    
    // Check file size
    size_t fileSize = getFileSize(inputPath);
    if (fileSize > MAX_FILE_SIZE) {
        std::cerr << "Error: File size exceeds maximum allowed size" << std::endl;
        return ChecksumResult(0, 0);
    }
    
    std::ifstream inputFile(inputPath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Cannot open input file" << std::endl;
        return ChecksumResult(0, 0);
    }
    
    // Read file contents
    std::vector<unsigned char> buffer(fileSize);
    inputFile.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    size_t bytesRead = inputFile.gcount();
    inputFile.close();
    
    // Calculate checksum
    unsigned long checksum = 0;
    for (size_t i = 0; i < bytesRead; i++) {
        checksum = (checksum + buffer[i]) & 0xFFFFFFFFUL; // Prevent overflow
    }
    
    // Write results to output file
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Cannot open output file" << std::endl;
        return ChecksumResult(0, 0);
    }
    
    outputFile << "Bytes Read: " << bytesRead << std::endl;
    outputFile << "Checksum: " << checksum << std::endl;
    outputFile << "Buffer Empty: " << (checksum == 0 ? "true" : "false") << std::endl;
    outputFile.close();
    
    return ChecksumResult(bytesRead, checksum);
}

int main() {
    // Test case 1: Normal file with content
    {
        std::ofstream f("test1.txt");
        f << "Hello World";
        f.close();
        ChecksumResult result = processFile("test1.txt", "output1.txt");
        std::cout << "Test 1 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 2: Empty file
    {
        std::ofstream f("test2.txt");
        f.close();
        ChecksumResult result = processFile("test2.txt", "output2.txt");
        std::cout << "Test 2 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 3: File with numbers
    {
        std::ofstream f("test3.txt");
        f << "1234567890";
        f.close();
        ChecksumResult result = processFile("test3.txt", "output3.txt");
        std::cout << "Test 3 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 4: Binary content
    {
        std::ofstream f("test4.txt", std::ios::binary);
        unsigned char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        f.write(reinterpret_cast<char*>(data), sizeof(data));
        f.close();
        ChecksumResult result = processFile("test4.txt", "output4.txt");
        std::cout << "Test 4 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 5: Non-existent file
    {
        ChecksumResult result = processFile("nonexistent.txt", "output5.txt");
        std::cout << "Test 5 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    return 0;
}
