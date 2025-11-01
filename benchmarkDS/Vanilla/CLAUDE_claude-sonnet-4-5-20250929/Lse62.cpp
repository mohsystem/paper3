
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct ChecksumResult {
    int bytesRead;
    long long checksum;
};

ChecksumResult processFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    ChecksumResult result = {0, 0};
    
    try {
        // Read file contents into buffer
        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error opening input file" << std::endl;
            return result;
        }
        
        // Get file size and read into buffer
        inputFile.seekg(0, std::ios::end);
        size_t fileSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        
        std::vector<unsigned char> buffer(fileSize);
        inputFile.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        result.bytesRead = inputFile.gcount();
        inputFile.close();
        
        // Calculate checksum
        result.checksum = 0;
        for (unsigned char byte : buffer) {
            result.checksum += byte;
        }
        
        // Check if checksum is 0 (buffer is empty)
        if (result.checksum == 0) {
            std::cout << "Buffer is empty or checksum is 0" << std::endl;
        }
        
        // Save results to output file
        std::ofstream outputFile(outputFilePath);
        if (outputFile) {
            outputFile << "Bytes Read: " << result.bytesRead << std::endl;
            outputFile << "Checksum: " << result.checksum << std::endl;
            outputFile.close();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return result;
}

int main() {
    // Test case 1: File with text content
    {
        std::ofstream test("test1.txt");
        test << "Hello World";
        test.close();
        ChecksumResult result = processFile("test1.txt", "output1.txt");
        std::cout << "Test 1 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 2: Empty file
    {
        std::ofstream test("test2.txt");
        test.close();
        ChecksumResult result = processFile("test2.txt", "output2.txt");
        std::cout << "Test 2 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 3: File with numbers
    {
        std::ofstream test("test3.txt");
        test << "1234567890";
        test.close();
        ChecksumResult result = processFile("test3.txt", "output3.txt");
        std::cout << "Test 3 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 4: File with special characters
    {
        std::ofstream test("test4.txt");
        test << "!@#$%^&*()";
        test.close();
        ChecksumResult result = processFile("test4.txt", "output4.txt");
        std::cout << "Test 4 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    // Test case 5: Larger text file
    {
        std::ofstream test("test5.txt");
        test << "The quick brown fox jumps over the lazy dog";
        test.close();
        ChecksumResult result = processFile("test5.txt", "output5.txt");
        std::cout << "Test 5 - Bytes: " << result.bytesRead << ", Checksum: " << result.checksum << std::endl;
    }
    
    return 0;
}
