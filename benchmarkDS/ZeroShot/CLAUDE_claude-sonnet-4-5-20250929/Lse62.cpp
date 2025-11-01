
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

class Lse62 {
public:
    struct ChecksumResult {
        long bytesRead;
        long checksum;
        
        ChecksumResult(long bytes = 0, long sum = 0) 
            : bytesRead(bytes), checksum(sum) {}
    };
    
    static ChecksumResult processFile(const std::string& inputFilePath, 
                                     const std::string& outputFilePath) {
        try {
            // Open input file
            std::ifstream inputFile(inputFilePath, std::ios::binary | std::ios::ate);
            if (!inputFile.is_open()) {
                std::cerr << "Error: Cannot open input file" << std::endl;
                return ChecksumResult(0, 0);
            }
            
            // Get file size
            std::streamsize fileSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);
            
            // Read file contents
            std::vector<unsigned char> buffer(fileSize);
            if (!inputFile.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
                std::cerr << "Error: Failed to read file" << std::endl;
                return ChecksumResult(0, 0);
            }
            inputFile.close();
            
            long bytesRead = fileSize;
            
            // Calculate checksum
            long checksum = 0;
            for (unsigned char byte : buffer) {
                checksum += byte;
            }
            
            // Save results to output file
            std::ofstream outputFile(outputFilePath);
            if (!outputFile.is_open()) {
                std::cerr << "Error: Cannot open output file" << std::endl;
                return ChecksumResult(bytesRead, checksum);
            }
            
            outputFile << "Bytes Read: " << bytesRead << std::endl;
            outputFile << "Checksum: " << checksum << std::endl;
            if (checksum == 0) {
                outputFile << "Status: Buffer is empty or all zeros" << std::endl;
            } else {
                outputFile << "Status: Buffer contains data" << std::endl;
            }
            outputFile.close();
            
            return ChecksumResult(bytesRead, checksum);
            
        } catch (const std::exception& e) {
            std::cerr << "Error processing file: " << e.what() << std::endl;
            return ChecksumResult(0, 0);
        }
    }
};

int main() {
    std::cout << "File Checksum Calculator - Test Cases\\n" << std::endl;
    
    // Test Case 1: Empty file
    {
        std::ofstream test1("test1_empty.txt");
        test1.close();
        auto result1 = Lse62::processFile("test1_empty.txt", "output1.txt");
        std::cout << "Test 1 - Empty file: Bytes=" << result1.bytesRead 
                  << ", Checksum=" << result1.checksum << std::endl;
        std::remove("test1_empty.txt");
    }
    
    // Test Case 2: Small text file
    {
        std::ofstream test2("test2_small.txt");
        test2 << "Hello";
        test2.close();
        auto result2 = Lse62::processFile("test2_small.txt", "output2.txt");
        std::cout << "Test 2 - Small file: Bytes=" << result2.bytesRead 
                  << ", Checksum=" << result2.checksum << std::endl;
        std::remove("test2_small.txt");
    }
    
    // Test Case 3: File with numbers
    {
        std::ofstream test3("test3_numbers.txt");
        test3 << "12345";
        test3.close();
        auto result3 = Lse62::processFile("test3_numbers.txt", "output3.txt");
        std::cout << "Test 3 - Numbers file: Bytes=" << result3.bytesRead 
                  << ", Checksum=" << result3.checksum << std::endl;
        std::remove("test3_numbers.txt");
    }
    
    // Test Case 4: File with special characters
    {
        std::ofstream test4("test4_special.txt");
        test4 << "@#$%^";
        test4.close();
        auto result4 = Lse62::processFile("test4_special.txt", "output4.txt");
        std::cout << "Test 4 - Special chars: Bytes=" << result4.bytesRead 
                  << ", Checksum=" << result4.checksum << std::endl;
        std::remove("test4_special.txt");
    }
    
    // Test Case 5: Larger text file
    {
        std::ofstream test5("test5_large.txt");
        test5 << "The quick brown fox jumps over the lazy dog.";
        test5.close();
        auto result5 = Lse62::processFile("test5_large.txt", "output5.txt");
        std::cout << "Test 5 - Larger file: Bytes=" << result5.bytesRead 
                  << ", Checksum=" << result5.checksum << std::endl;
        std::remove("test5_large.txt");
    }
    
    return 0;
}
