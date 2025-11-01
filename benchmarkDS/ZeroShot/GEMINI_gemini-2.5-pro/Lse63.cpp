#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint> // For uint64_t
#include <numeric> // For std::accumulate

// Define a reasonable maximum file size to prevent DoS attacks (e.g., 100 MB)
const long long MAX_FILE_SIZE = 100 * 1024 * 1024;

/**
 * @brief Reads a file, calculates a checksum, and writes the results to another file.
 *
 * @param inputPath The path to the input file.
 * @param outputPath The path to the output file.
 * @return true on success, false on failure.
 */
bool processFile(const std::string& inputPath, const std::string& outputPath) {
    // 1. Open the input file for reading in binary mode
    // Use RAII (std::ifstream) to ensure the file is closed automatically
    std::ifstream inFile(inputPath, std::ios::binary | std::ios::ate);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file: " << inputPath << std::endl;
        return false;
    }

    // 2. Securely check file size before reading
    std::streamsize fileSize = inFile.tellg();
    if (fileSize > MAX_FILE_SIZE) {
        std::cerr << "Error: Input file is too large: " << fileSize << " bytes. Max allowed: " << MAX_FILE_SIZE << " bytes." << std::endl;
        return false;
    }
    inFile.seekg(0, std::ios::beg);

    // 3. Read file contents into a buffer
    std::vector<unsigned char> buffer(fileSize);
    if (fileSize > 0) {
        if (!inFile.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
            std::cerr << "Error: Could not read from input file: " << inputPath << std::endl;
            return false;
        }
    }

    // 4. Calculate checksum
    uint64_t checksum = 0;
    for (unsigned char b : buffer) {
        checksum += b;
    }
    
    if (buffer.empty()) {
        std::cout << "Info: Buffer is empty for file " << inputPath << std::endl;
    }

    // 5. Open the output file for writing
    // Use RAII (std::ofstream) to ensure the file is closed automatically
    std::ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for writing: " << outputPath << std::endl;
        return false;
    }

    outFile << "Bytes read: " << buffer.size() << "\n";
    outFile << "Checksum: " << checksum << "\n";

    if (!outFile) {
        std::cerr << "Error: Failed to write to output file: " << outputPath << std::endl;
        return false;
    }

    return true;
}

// Main function with 5 test cases
void runTest(const std::string& testName, const std::string& in, const std::string& out, bool expected) {
    bool result = processFile(in, out);
    if (result == expected) {
        std::cout << "  " << testName << ": PASSED" << std::endl;
    } else {
        std::cout << "  " << testName << ": FAILED (Expected " << (expected ? "true" : "false") << ", got " << (result ? "true" : "false") << ")" << std::endl;
    }
}

void createTestFile(const std::string& path, const std::vector<unsigned char>& content) {
    std::ofstream file(path, std::ios::binary);
    if (file.is_open() && !content.empty()) {
        file.write(reinterpret_cast<const char*>(content.data()), content.size());
    }
}

void cleanupTestFiles(const std::vector<std::string>& files) {
    for (const auto& file : files) {
        std::remove(file.c_str());
    }
}


int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Empty file
    std::string emptyIn = "test_empty_in.txt", emptyOut = "test_empty_out.txt";
    createTestFile(emptyIn, {});
    std::cout << "Test 1: Empty File" << std::endl;
    runTest("Test 1", emptyIn, emptyOut, true);
    
    // Test Case 2: ASCII text file
    std::string asciiIn = "test_ascii_in.txt", asciiOut = "test_ascii_out.txt";
    std::string asciiContent = "Hello World!";
    createTestFile(asciiIn, std::vector<unsigned char>(asciiContent.begin(), asciiContent.end()));
    std::cout << "Test 2: ASCII File" << std::endl;
    runTest("Test 2", asciiIn, asciiOut, true);

    // Test Case 3: Binary file
    std::string binaryIn = "test_binary_in.txt", binaryOut = "test_binary_out.txt";
    createTestFile(binaryIn, {0x01, 0x02, 0x00, 0xFF, 0xFE});
    std::cout << "Test 3: Binary File" << std::endl;
    runTest("Test 3", binaryIn, binaryOut, true);

    // Test Case 4: Non-existent input file
    std::string nonExistentIn = "non_existent_file.txt", nonExistentOut = "test_no_input_out.txt";
    std::cout << "Test 4: Non-existent Input File" << std::endl;
    runTest("Test 4", nonExistentIn, nonExistentOut, false);
    
    // Test Case 5: Unwritable output file (invalid path)
    std::string goodIn = asciiIn; // Re-use valid input
    std::string unwritableOut = "non_existent_dir/output.txt";
    std::cout << "Test 5: Unwritable Output Path" << std::endl;
    runTest("Test 5", goodIn, unwritableOut, false);
    
    // Cleanup
    cleanupTestFiles({emptyIn, emptyOut, asciiIn, asciiOut, binaryIn, binaryOut, nonExistentOut});

    std::cout << "--- C++ Test Cases Finished ---" << std::endl << std::endl;
    return 0;
}