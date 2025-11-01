
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct Result {
    int bytesRead;
    int checksum;
};

Result processFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    Result result = {0, 0};
    
    // Read file contents into buffer
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error opening input file" << std::endl;
        return result;
    }
    
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(inputFile)),
                                       std::istreambuf_iterator<char>());
    inputFile.close();
    
    result.bytesRead = buffer.size();
    
    // Calculate checksum
    result.checksum = 0;
    for (unsigned char byte : buffer) {
        result.checksum += byte;
    }
    
    // Write results to output file
    std::ofstream outputFile(outputFilePath);
    if (outputFile) {
        outputFile << "Bytes Read: " << result.bytesRead << std::endl;
        outputFile << "Checksum: " << result.checksum << std::endl;
        outputFile.close();
    }
    
    return result;
}

int main() {
    // Test case 1: Empty file
    {
        std::ofstream f("test1.txt", std::ios::binary);
        f.close();
        Result r1 = processFile("test1.txt", "output1.txt");
        std::cout << "Test 1 - Bytes: " << r1.bytesRead << ", Checksum: " << r1.checksum << std::endl;
    }
    
    // Test case 2: Small text file
    {
        std::ofstream f("test2.txt", std::ios::binary);
        f << "Hello";
        f.close();
        Result r2 = processFile("test2.txt", "output2.txt");
        std::cout << "Test 2 - Bytes: " << r2.bytesRead << ", Checksum: " << r2.checksum << std::endl;
    }
    
    // Test case 3: Numbers
    {
        std::ofstream f("test3.txt", std::ios::binary);
        f << "12345";
        f.close();
        Result r3 = processFile("test3.txt", "output3.txt");
        std::cout << "Test 3 - Bytes: " << r3.bytesRead << ", Checksum: " << r3.checksum << std::endl;
    }
    
    // Test case 4: Special characters
    {
        std::ofstream f("test4.txt", std::ios::binary);
        f << "!@#$%";
        f.close();
        Result r4 = processFile("test4.txt", "output4.txt");
        std::cout << "Test 4 - Bytes: " << r4.bytesRead << ", Checksum: " << r4.checksum << std::endl;
    }
    
    // Test case 5: Binary data
    {
        std::ofstream f("test5.txt", std::ios::binary);
        unsigned char data[] = {1, 2, 3, 4, 5, 255, 254, 253};
        f.write((char*)data, sizeof(data));
        f.close();
        Result r5 = processFile("test5.txt", "output5.txt");
        std::cout << "Test 5 - Bytes: " << r5.bytesRead << ", Checksum: " << r5.checksum << std::endl;
    }
    
    return 0;
}
