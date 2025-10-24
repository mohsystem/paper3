#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/**
 * @brief Reads a file specified by the file path and prints its contents to the console.
 * @param filePath The path to the file to be read.
 */
void readFileAndPrint(const std::string& filePath) {
    std::cout << "Reading file: " << filePath << std::endl;
    std::ifstream inputFile(filePath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open file '" << filePath << "'" << std::endl;
        return;
    }

    std::string line;
    // Check if the file is empty before trying to read
    if (inputFile.peek() == std::ifstream::traits_type::eof()) {
        std::cout << "(File is empty)" << std::endl;
    } else {
        while (std::getline(inputFile, line)) {
            std::cout << line << std::endl;
        }
    }

    inputFile.close();
}

/**
 * @brief Helper function to create a test file with specified content.
 */
void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    } else {
        std::cerr << "Failed to create test file: " << fileName << std::endl;
    }
}

int main() {
    // --- Test Case 1: Read a simple, single-line file ---
    std::cout << "--- Test Case 1: Simple File ---" << std::endl;
    std::string file1 = "cpp_test1.txt";
    createTestFile(file1, "Hello, World from C++!");
    readFileAndPrint(file1);
    std::cout << "-------------------------------------" << std::endl << std::endl;

    // --- Test Case 2: Read a multi-line file ---
    std::cout << "--- Test Case 2: Multi-line File ---" << std::endl;
    std::string file2 = "cpp_test2.txt";
    createTestFile(file2, "This is line one.\nThis is line two.\nAnd this is the third line.");
    readFileAndPrint(file2);
    std::cout << "-------------------------------------" << std::endl << std::endl;

    // --- Test Case 3: Read an empty file ---
    std::cout << "--- Test Case 3: Empty File ---" << std::endl;
    std::string file3 = "cpp_test3_empty.txt";
    createTestFile(file3, "");
    readFileAndPrint(file3);
    std::cout << "-------------------------------------" << std::endl << std::endl;

    // --- Test Case 4: Attempt to read a non-existent file ---
    std::cout << "--- Test Case 4: Non-existent File ---" << std::endl;
    readFileAndPrint("nonexistentfile.txt");
    std::cout << "-------------------------------------" << std::endl << std::endl;

    // --- Test Case 5: Read a file with special characters ---
    std::cout << "--- Test Case 5: Special Characters File ---" << std::endl;
    std::string file4 = "cpp_test4_special.txt";
    // Note: C++ console support for UTF-8 can be platform-dependent.
    createTestFile(file4, "Special chars: ñ, é, ö, ü, ç, α, β, γ.");
    readFileAndPrint(file4);
    std::cout << "-------------------------------------" << std::endl << std::endl;

    // Cleanup test files
    remove(file1.c_str());
    remove(file2.c_str());
    remove(file3.c_str());
    remove(file4.c_str());

    return 0;
}