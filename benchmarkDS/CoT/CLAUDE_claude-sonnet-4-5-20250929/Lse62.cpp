
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <limits>

const size_t MAX_BUFFER_SIZE = 1024 * 1024; // 1MB limit

struct ChecksumResult {
    size_t bytesRead;
    unsigned long long checksum;
    
    ChecksumResult(size_t bytes, unsigned long long sum) 
        : bytesRead(bytes), checksum(sum) {}
};

unsigned long long calculateChecksum(const std::vector<unsigned char>& buffer) {
    unsigned long long checksum = 0;
    for (unsigned char byte : buffer) {
        checksum += static_cast<unsigned long long>(byte);
    }
    return checksum;
}

ChecksumResult processFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    if (inputFilePath.empty() || outputFilePath.empty()) {
        std::cerr << "Error: File paths cannot be empty" << std::endl;
        return ChecksumResult(0, 0);
    }
    
    std::ifstream inputFile(inputFilePath, std::ios::binary | std::ios::ate);
    if (!inputFile) {
        std::cerr << "Error: Cannot open input file: " << inputFilePath << std::endl;
        return ChecksumResult(0, 0);
    }
    
    // Get file size
    std::streamsize fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);
    
    if (fileSize < 0 || static_cast<size_t>(fileSize) > MAX_BUFFER_SIZE) {
        std::cerr << "Error: File too large or invalid. Maximum size: " << MAX_BUFFER_SIZE << std::endl;
        inputFile.close();
        return ChecksumResult(0, 0);
    }
    
    // Read file into buffer
    std::vector<unsigned char> buffer(static_cast<size_t>(fileSize));
    if (!inputFile.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        std::cerr << "Error: Failed to read file" << std::endl;
        inputFile.close();
        return ChecksumResult(0, 0);
    }
    inputFile.close();
    
    size_t bytesRead = buffer.size();
    
    // Calculate checksum
    unsigned long long checksum = calculateChecksum(buffer);
    
    // Save results to output file
    std::ofstream outputFile(outputFilePath);
    if (!outputFile) {
        std::cerr << "Error: Cannot open output file: " << outputFilePath << std::endl;
        return ChecksumResult(bytesRead, checksum);
    }
    
    outputFile << "Bytes Read: " << bytesRead << "\\n";
    outputFile << "Checksum: " << checksum << "\\n";
    if (checksum == 0) {
        outputFile << "Status: Buffer is empty or checksum is zero\\n";
    }
    outputFile.close();
    
    return ChecksumResult(bytesRead, checksum);
}

void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (file) {
        file << content;
        file.close();
    } else {
        std::cerr << "Error creating test file: " << filename << std::endl;
    }
}

int main() {
    // Test case 1: Normal file
    createTestFile("test1.txt", "Hello World");
    processFile("test1.txt", "output1.txt");
    
    // Test case 2: Empty file
    createTestFile("test2.txt", "");
    processFile("test2.txt", "output2.txt");
    
    // Test case 3: File with numbers
    createTestFile("test3.txt", "1234567890");
    processFile("test3.txt", "output3.txt");
    
    // Test case 4: Binary-like content
    createTestFile("test4.txt", std::string(4, '\\0'));
    processFile("test4.txt", "output4.txt");
    
    // Test case 5: Larger text
    createTestFile("test5.txt", "The quick brown fox jumps over the lazy dog");
    processFile("test5.txt", "output5.txt");
    
    std::cout << "All test cases executed. Check output files." << std::endl;
    
    return 0;
}
