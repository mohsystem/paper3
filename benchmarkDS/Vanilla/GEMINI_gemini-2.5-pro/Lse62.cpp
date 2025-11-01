#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>

/**
 * Reads the contents of a file into a buffer, calculates the checksum of the buffer,
 * and saves the number of bytes read and the checksum to another file.
 *
 * @param inputFilePath  The path to the file to be read.
 * @param outputFilePath The path to the file where results will be saved.
 */
void processFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    long long checksum = 0;
    size_t bytesRead = 0;
    std::vector<unsigned char> buffer;

    std::ifstream inputFile(inputFilePath, std::ios::binary | std::ios::ate);
    
    if (inputFile.is_open()) {
        std::streamsize size = inputFile.tellg();
        if (size > 0) {
            inputFile.seekg(0, std::ios::beg);
            buffer.resize(size);
            if (inputFile.read(reinterpret_cast<char*>(buffer.data()), size)) {
                bytesRead = buffer.size();
                // Calculate checksum by summing all byte values
                for (unsigned char byte : buffer) {
                    checksum += byte;
                }
            }
        }
        inputFile.close();
    }
    // If file doesn't exist or is empty, bytesRead and checksum remain 0.

    std::ofstream outputFile(outputFilePath);
    if (outputFile.is_open()) {
        outputFile << "Bytes read: " << bytesRead << "\n";
        outputFile << "Checksum: " << checksum << "\n";
        outputFile.close();
    } else {
        std::cerr << "Error: Could not open output file " << outputFilePath << std::endl;
    }
}

// Helper function to create a test file
void createTestFile(const std::string& path, const std::vector<unsigned char>& content) {
    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (file.is_open()) {
        if (!content.empty()) {
            file.write(reinterpret_cast<const char*>(content.data()), content.size());
        }
        file.close();
    }
}

// Helper function to read and print the content of an output file for verification
void printOutputFile(const std::string& path) {
    std::ifstream file(path);
    if (file.is_open()) {
        std::cout << "--- Contents of " << path << " ---" << std::endl;
        std::cout << file.rdbuf();
        std::cout << "------------------------------------" << std::endl;
        file.close();
    } else {
        std::cerr << "Could not read output file " << path << std::endl;
    }
}

int main() {
    // Test Case 1: A non-empty text file
    const std::string test1_in = "test1_in.txt";
    const std::string test1_out = "test1_out.txt";
    std::vector<unsigned char> content1 = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    createTestFile(test1_in, content1);
    processFile(test1_in, test1_out);
    printOutputFile(test1_out);

    // Test Case 2: An empty file
    const std::string test2_in = "test2_in.txt";
    const std::string test2_out = "test2_out.txt";
    createTestFile(test2_in, {});
    processFile(test2_in, test2_out);
    printOutputFile(test2_out);

    // Test Case 3: A file with binary data
    const std::string test3_in = "test3_in.bin";
    const std::string test3_out = "test3_out.bin";
    std::vector<unsigned char> binaryData = {0xDE, 0xAD, 0xBE, 0xEF};
    createTestFile(test3_in, binaryData);
    processFile(test3_in, test3_out);
    printOutputFile(test3_out);

    // Test Case 4: A file with a single byte
    const std::string test4_in = "test4_in.txt";
    const std::string test4_out = "test4_out.txt";
    createTestFile(test4_in, {'A'});
    processFile(test4_in, test4_out);
    printOutputFile(test4_out);

    // Test Case 5: A file that doesn't exist
    const std::string test5_in = "non_existent_file.txt";
    const std::string test5_out = "test5_out.txt";
    remove(test5_in.c_str());
    processFile(test5_in, test5_out);
    printOutputFile(test5_out);

    return 0;
}