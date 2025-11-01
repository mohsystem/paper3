#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <cstdio> // For remove()

// For creating directories in test cases, use filesystem if available (C++17+)
#if __cplusplus >= 201703L && defined(__has_include)
#if __has_include(<filesystem>)
#include <filesystem>
#define CAN_CREATE_DIR 1
#endif
#endif

/**
 * @brief Reads a file, calculates its checksum, and writes results to an output file.
 *
 * Uses modern C++ features like RAII for secure resource management. Reads the
 * file in chunks to handle large files efficiently.
 *
 * @param inputFilePath Path to the input file.
 * @param outputFilePath Path to the output file.
 * @return true on success, false on failure.
 */
bool processFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    // Open in binary mode for consistent byte reading.
    // ifstream's destructor will close the file automatically (RAII).
    std::ifstream inFile(inputFilePath, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file: " << inputFilePath << std::endl;
        return false;
    }

    const size_t BUFFER_SIZE = 4096;
    // Using std::vector is safer than raw C-style arrays.
    std::vector<char> buffer(BUFFER_SIZE);
    unsigned long long numberOfBytesRead = 0;
    unsigned long long checksum = 0;

    // Read file in chunks
    while (inFile) {
        inFile.read(buffer.data(), BUFFER_SIZE);
        std::streamsize bytesReadInChunk = inFile.gcount();
        if (bytesReadInChunk > 0) {
            numberOfBytesRead += bytesReadInChunk;
            for (std::streamsize i = 0; i < bytesReadInChunk; ++i) {
                // Cast to unsigned char to prevent sign extension before adding to checksum.
                checksum += static_cast<unsigned char>(buffer[i]);
            }
        }
    }
    // No need for explicit close(), but it's fine to have.
    inFile.close();

    if (checksum == 0) {
        std::cout << "Buffer is empty for file: " << inputFilePath << std::endl;
    }

    // ofstream's destructor will also close the file automatically.
    std::ofstream outFile(outputFilePath);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for writing: " << outputFilePath << std::endl;
        return false;
    }

    outFile << "Bytes read: " << numberOfBytesRead << "\n";
    outFile << "Checksum: " << checksum << "\n";
    outFile.close();

    return true;
}

void setupTestFiles() {
    std::ofstream("test1.txt", std::ios::binary).close(); // Empty
    std::ofstream("test2.txt") << "Hello"; // Checksum: 500
    std::ofstream("test3.bin", std::ios::binary).write("\x01\x02\xFF", 3); // Checksum: 258
}

void runTestCase(const std::string& inputFile, const std::string& outputFile) {
    std::cout << "Processing " << inputFile << " -> " << outputFile << std::endl;
    bool success = processFile(inputFile, outputFile);
    std::cout << "Result: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
    if (success) {
        std::ifstream resultFile(outputFile);
        if (resultFile.is_open()) {
            std::cout << "Output content:\n---\n" << resultFile.rdbuf() << "---\n";
        }
    }
}

void cleanupTestFiles() {
    remove("test1.txt");
    remove("test2.txt");
    remove("test3.bin");
    remove("output1.txt");
    remove("output2.txt");
    remove("output3.txt");
#ifdef CAN_CREATE_DIR
    std::filesystem::remove("output5.txt");
#else
    remove("output5.txt"); // This will fail for a directory, but we try.
#endif
}

int main() {
    setupTestFiles();

    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Empty file
    std::cout << "\nTest Case 1: Empty File" << std::endl;
    runTestCase("test1.txt", "output1.txt");

    // Test Case 2: Small text file
    std::cout << "\nTest Case 2: Small Text File" << std::endl;
    runTestCase("test2.txt", "output2.txt");
    
    // Test Case 3: Small binary file
    std::cout << "\nTest Case 3: Small Binary File" << std::endl;
    runTestCase("test3.bin", "output3.txt");
    
    // Test Case 4: Non-existent input file
    std::cout << "\nTest Case 4: Non-existent Input File" << std::endl;
    runTestCase("nonexistent.txt", "output4.txt");

    // Test Case 5: Unwritable output file (simulated by using a directory)
    std::cout << "\nTest Case 5: Unwritable Output File" << std::endl;
#ifdef CAN_CREATE_DIR
    std::filesystem::create_directory("output5.txt");
    runTestCase("test1.txt", "output5.txt");
#else
    std::cout << "Skipping Test Case 5: Requires C++17 <filesystem> to create a directory for the test." << std::endl;
#endif
    
    std::cout << "\n--- Cleaning up test files ---" << std::endl;
    cleanupTestFiles();
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}