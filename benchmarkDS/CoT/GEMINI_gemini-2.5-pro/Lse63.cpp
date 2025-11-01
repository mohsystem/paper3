#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>

/**
 * @brief Reads a file into a buffer, calculates an 8-bit checksum,
 * and writes the number of bytes read and the checksum to an output file.
 *
 * @param inputFilePath The path to the input file.
 * @param outputFilePath The path to the output file.
 * @return true on success, false on failure.
 */
bool processFileAndGetChecksum(const std::string& inputFilePath, const std::string& outputFilePath) {
    // 1. Read the contents of a file into a buffer.
    std::ifstream inputFile(inputFilePath, std::ios::binary | std::ios::ate);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open input file: " << inputFilePath << std::endl;
        return false;
    }

    std::streamsize bytesRead = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(bytesRead);
    if (bytesRead > 0) {
        if (!inputFile.read(buffer.data(), bytesRead)) {
            std::cerr << "Error: Could not read from input file: " << inputFilePath << std::endl;
            inputFile.close();
            return false;
        }
    }
    inputFile.close();

    // 2. Calculate the checksum of the buffer.
    int checksum = 0;
    for (const auto& byte : buffer) {
        // Cast to unsigned char to ensure values are 0-255
        checksum = (checksum + static_cast<unsigned char>(byte)) % 256;
    }

    // 3. Save the number of bytes read and the checksum to another file.
    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file: " << outputFilePath << std::endl;
        return false;
    }

    outputFile << "Bytes read: " << bytesRead << ", Checksum: " << checksum;
    outputFile.close();

    return true;
}

// Helper function to create a test file
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename, std::ios::binary);
    file.write(content.c_str(), content.length());
    file.close();
}

// Helper function to read a file for verification
std::string readTestFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return "Could not read file";
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

// Main function with 5 test cases
int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Standard text file
    createTestFile("test1_in.txt", "Hello, World!");
    if (processFileAndGetChecksum("test1_in.txt", "test1_out.txt")) {
        std::cout << "Test Case 1 (Text): " << readTestFile("test1_out.txt") << std::endl;
    }
    remove("test1_in.txt");
    remove("test1_out.txt");

    // Test Case 2: Binary data
    createTestFile("test2_in.bin", std::string("\xDE\xAD\xBE\xEF", 4));
    if (processFileAndGetChecksum("test2_in.bin", "test2_out.txt")) {
        std::cout << "Test Case 2 (Binary): " << readTestFile("test2_out.txt") << std::endl;
    }
    remove("test2_in.bin");
    remove("test2_out.txt");
    
    // Test Case 3: Empty file
    createTestFile("test3_in.txt", "");
    if (processFileAndGetChecksum("test3_in.txt", "test3_out.txt")) {
        std::cout << "Test Case 3 (Empty): " << readTestFile("test3_out.txt") << std::endl;
    }
    remove("test3_in.txt");
    remove("test3_out.txt");

    // Test Case 4: Non-existent file
    std::cout << "Test Case 4 (Not Found): ";
    processFileAndGetChecksum("non_existent_file.txt", "test4_out.txt");

    // Test Case 5: File with content that results in a zero checksum
    createTestFile("test5_in.bin", std::string("\x01\xFF", 2));
    if (processFileAndGetChecksum("test5_in.bin", "test5_out.txt")) {
        std::cout << "Test Case 5 (Zero Checksum): " << readTestFile("test5_out.txt") << std::endl;
    }
    remove("test5_in.bin");
    remove("test5_out.txt");

    std::cout << "--- C++ Tests Complete ---" << std::endl << std::endl;
    return 0;
}