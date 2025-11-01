#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric> // For std::accumulate

// Using a class structure to match the Java example name 'Lse62'
class Lse62 {
public:
    /**
     * Reads a file into a buffer, calculates a checksum, and saves the byte count
     * and checksum to an output file.
     *
     * @param inputFilePath The path to the input file.
     * @param outputFilePath The path to the output file where results will be saved.
     * @return true on success, false on failure.
     */
    static bool processFileAndCalculateChecksum(const std::string& inputFilePath, const std::string& outputFilePath) {
        // 1. Read the contents of the file into a buffer.
        // Open the file in binary mode. The ifstream object will handle closing the file
        // automatically when it goes out of scope (RAII).
        std::ifstream inputFile(inputFilePath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Error: Could not open input file: " << inputFilePath << std::endl;
            return false;
        }

        // Read the entire file content into a vector of chars.
        // This method avoids reading byte by byte and is efficient for moderate file sizes.
        std::vector<char> buffer((std::istreambuf_iterator<char>(inputFile)),
                                 std::istreambuf_iterator<char>());
        
        // After reading, inputFile can be implicitly closed.
        inputFile.close();

        // 2. Calculate the checksum of the buffer.
        // We use a long long to prevent overflow. We must cast the char to an unsigned
        // char to ensure the values are treated as 0-255.
        long long checksum = 0;
        for (const char& c : buffer) {
            checksum += static_cast<unsigned char>(c);
        }

        long long bytesRead = buffer.size();

        // 3. Save the number of bytes read and the checksum to a file.
        std::ofstream outputFile(outputFilePath);
        if (!outputFile.is_open()) {
            std::cerr << "Error: Could not open output file for writing: " << outputFilePath << std::endl;
            return false;
        }

        outputFile << "Bytes: " << bytesRead << ", Checksum: " << checksum;
        outputFile.close();

        return true;
    }
};

void run_test(int testNum, const std::string& testName, const std::string& inputFile, const std::string& outputFile, const std::string& fileContent = "", bool binary = false) {
    std::cout << "--- Test Case " << testNum << ": " << testName << " ---" << std::endl;
    
    // Setup: create input file if content is provided
    if (!fileContent.empty()) {
        std::ofstream testFile(inputFile, binary ? std::ios::binary : std::ios::out);
        testFile << fileContent;
        testFile.close();
    }

    bool success = Lse62::processFileAndCalculateChecksum(inputFile, outputFile);
    std::cout << "Test " << testNum << " Success: " << std::boolalpha << success << std::endl;
    
    if (success) {
        std::ifstream resultFile(outputFile);
        if(resultFile.is_open()) {
            std::string line;
            std::getline(resultFile, line);
            std::cout << "Output: " << line << std::endl;
        }
    }

    // Cleanup
    remove(inputFile.c_str());
    remove(outputFile.c_str());

    std::cout << std::endl;
}

int main() {
    // Test Case 1: A normal text file.
    run_test(1, "Normal File", "test_input_1.txt", "test_output_1.txt", "Hello"); // Expected: Bytes: 5, Checksum: 500

    // Test Case 2: An empty file.
    run_test(2, "Empty File", "test_input_2.txt", "test_output_2.txt", ""); // Expected: Bytes: 0, Checksum: 0

    // Test Case 3: A binary file.
    std::string binaryContent;
    binaryContent += (char)0x01;
    binaryContent += (char)0x80;
    binaryContent += (char)0xFF;
    run_test(3, "Binary File", "test_input_3.bin", "test_output_3.txt", binaryContent, true); // Expected: Bytes: 3, Checksum: 384
    
    // Test Case 4: Non-existent input file.
    run_test(4, "Non-existent Input File", "non_existent_file.txt", "test_output_4.txt"); // Expected: false

    // Test Case 5: Unwritable output file path.
    // Using a path that includes a non-existent directory.
    std::cout << "--- Test Case 5: Unwritable Output Path ---" << std::endl;
    std::string inputFile5 = "test_input_5.txt";
    std::string outputFile5 = "non_existent_dir/test_output_5.txt";
    // Setup
    std::ofstream testFile5(inputFile5);
    testFile5 << "test";
    testFile5.close();
    bool success5 = Lse62::processFileAndCalculateChecksum(inputFile5, outputFile5);
    std::cout << "Test 5 Success: " << std::boolalpha << success5 << std::endl; // Expected: false
    // Cleanup
    remove(inputFile5.c_str());
    std::cout << std::endl;
    
    return 0;
}