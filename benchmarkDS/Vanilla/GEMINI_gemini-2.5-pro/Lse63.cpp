#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <map>
#include <cstdio> // For remove()

class Lse63 {
public:
    /**
     * Reads the contents of a file into a buffer, calculates a simple checksum,
     * and saves the number of bytes read and the checksum to another file.
     *
     * @param inputFilePath  The path to the input file.
     * @param outputFilePath The path where the results will be saved.
     */
    static void processFile(const std::string& inputFilePath, const std::string& outputFilePath) {
        std::ifstream inputFile(inputFilePath, std::ios::binary | std::ios::ate);
        if (!inputFile.is_open()) {
            std::cerr << "Error: Could not open input file " << inputFilePath << std::endl;
            return;
        }

        std::streamsize size = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (size > 0 && !inputFile.read(buffer.data(), size)) {
            std::cerr << "Error: Could not read from file " << inputFilePath << std::endl;
            inputFile.close();
            return;
        }
        inputFile.close();

        long bytesRead = buffer.size();
        int checksum = 0;

        // If the buffer is empty, bytesRead is 0, and the checksum remains 0.
        if (bytesRead > 0) {
            checksum = std::accumulate(buffer.begin(), buffer.end(), 0,
                [](int sum, char c) {
                    return sum + static_cast<unsigned char>(c);
                });
            checksum %= 256;
        }
        
        std::ofstream outputFile(outputFilePath);
        if (!outputFile.is_open()) {
            std::cerr << "Error: Could not open output file " << outputFilePath << std::endl;
            return;
        }

        outputFile << "Bytes read: " << bytesRead << "\n"
                   << "Checksum: " << checksum;
        outputFile.close();
    }

    static void runTests() {
        std::map<std::string, std::vector<char>> testCases;
        testCases["test1_in.txt"] = {}; // Empty file
        testCases["test2_in.txt"] = {'A'}; // Single character (ASCII 65)
        testCases["test3_in.txt"] = {'H', 'e', 'l', 'l', 'o'}; // Simple string
        testCases["test4_in.txt"] = {(char)0x80, (char)0x80}; // Non-empty, checksum 0 (128+128=256)
        testCases["test5_in.txt"] = {'\0'}; // Null byte

        int testNum = 1;
        for (const auto& pair : testCases) {
            std::string inputFileName = pair.first;
            const auto& content = pair.second;
            std::string outputFileName = "test" + std::to_string(testNum) + "_out.txt";
            
            std::cout << "--- Test Case " << testNum << " ---" << std::endl;
            std::cout << "Input File: " << inputFileName << std::endl;

            // 1. Create input file with specified content
            std::ofstream tempInputFile(inputFileName, std::ios::binary);
            tempInputFile.write(content.data(), content.size());
            tempInputFile.close();

            // 2. Process the file
            processFile(inputFileName, outputFileName);

            // 3. Read and display the output file content for verification
            std::ifstream resultFile(outputFileName);
            if (resultFile.is_open()) {
                std::cout << "Output from " << outputFileName << ":" << std::endl;
                std::cout << resultFile.rdbuf();
                resultFile.close();
            } else {
                std::cout << "Output file " << outputFileName << " was not created." << std::endl;
            }

            // 4. Clean up created files
            remove(inputFileName.c_str());
            remove(outputFileName.c_str());

            std::cout << "\n---------------------\n" << std::endl;
            testNum++;
        }
    }
};

int main() {
    Lse63::runTests();
    return 0;
}