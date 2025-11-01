
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

struct ChecksumResult {
    int bytesRead;
    long checksum;
    
    ChecksumResult(int br = 0, long cs = 0) : bytesRead(br), checksum(cs) {}
};

long calculateChecksum(const std::vector<unsigned char>& buffer) {
    long checksum = 0;
    for (unsigned char byte : buffer) {
        checksum += byte;
    }
    return checksum;
}

void saveResults(const std::string& outputPath, int bytesRead, long checksum) {
    std::ofstream outFile(outputPath);
    if (outFile.is_open()) {
        outFile << "Bytes Read: " << bytesRead << std::endl;
        outFile << "Checksum: " << checksum << std::endl;
        outFile.close();
    }
}

ChecksumResult processFile(const std::string& inputPath, const std::string& outputPath) {
    try {
        // Read file contents into buffer
        std::ifstream inFile(inputPath, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error opening file: " << inputPath << std::endl;
            return ChecksumResult(0, 0);
        }
        
        std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(inFile)),
                                          std::istreambuf_iterator<char>());
        inFile.close();
        
        int bytesRead = buffer.size();
        
        // Calculate checksum
        long checksum = calculateChecksum(buffer);
        
        // Check if buffer is empty (checksum is 0)
        if (checksum == 0) {
            std::cout << "Buffer is empty or checksum is 0" << std::endl;
        }
        
        // Save results to output file
        saveResults(outputPath, bytesRead, checksum);
        
        return ChecksumResult(bytesRead, checksum);
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing file: " << e.what() << std::endl;
        return ChecksumResult(0, 0);
    }
}

int main() {
    // Test case 1: File with some content
    {
        std::ofstream testFile("test1.txt");
        testFile << "Hello World";
        testFile.close();
        ChecksumResult result1 = processFile("test1.txt", "output1.txt");
        std::cout << "Test 1 - Bytes: " << result1.bytesRead << ", Checksum: " << result1.checksum << std::endl;
    }
    
    // Test case 2: Empty file
    {
        std::ofstream testFile("test2.txt");
        testFile << "";
        testFile.close();
        ChecksumResult result2 = processFile("test2.txt", "output2.txt");
        std::cout << "Test 2 - Bytes: " << result2.bytesRead << ", Checksum: " << result2.checksum << std::endl;
    }
    
    // Test case 3: File with numbers
    {
        std::ofstream testFile("test3.txt");
        testFile << "1234567890";
        testFile.close();
        ChecksumResult result3 = processFile("test3.txt", "output3.txt");
        std::cout << "Test 3 - Bytes: " << result3.bytesRead << ", Checksum: " << result3.checksum << std::endl;
    }
    
    // Test case 4: File with special characters
    {
        std::ofstream testFile("test4.txt");
        testFile << "!@#$%^&*()";
        testFile.close();
        ChecksumResult result4 = processFile("test4.txt", "output4.txt");
        std::cout << "Test 4 - Bytes: " << result4.bytesRead << ", Checksum: " << result4.checksum << std::endl;
    }
    
    // Test case 5: File with longer text
    {
        std::ofstream testFile("test5.txt");
        testFile << "The quick brown fox jumps over the lazy dog";
        testFile.close();
        ChecksumResult result5 = processFile("test5.txt", "output5.txt");
        std::cout << "Test 5 - Bytes: " << result5.bytesRead << ", Checksum: " << result5.checksum << std::endl;
    }
    
    return 0;
}
