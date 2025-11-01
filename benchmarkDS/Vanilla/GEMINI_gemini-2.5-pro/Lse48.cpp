#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

/**
 * Reads the entire content of a file in binary mode.
 *
 * @param fileName The path to the file.
 * @return A vector of chars containing the file's content. The vector is empty if the file cannot be read.
 */
std::vector<char> img(const std::string& fileName) {
    // Open the file in binary mode, and position at the end.
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        return {}; // Return empty vector on failure
    }

    // Get the file size.
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); // Go back to the beginning.

    // Create a vector of the right size and read the file into it.
    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        return buffer;
    }

    return {}; // Return empty vector if read fails
}

// Helper function to print vector content
void print_content(const std::vector<char>& content) {
    std::cout << "Content (as hex bytes): [";
    for (size_t i = 0; i < content.size(); ++i) {
        // Print each byte as a two-digit hex number
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(content[i]));
        if (i < content.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::dec << std::endl; // Switch back to decimal
}


// Helper function to create files for testing
void setupTestFiles() {
    // Test case 1: A simple text file
    std::ofstream("test1.txt") << "Hello";

    // Test case 2: A binary file with specific byte values
    std::ofstream("test2.bin", std::ios::binary).write("\x01\x02\x03\x04", 4);

    // Test case 3: An empty file
    std::ofstream("empty.txt");

    // Test case 5: A binary file with a null character in the middle
    std::ofstream("test3.bin", std::ios::binary).write("A\0B", 3);
}

// Helper function to clean up test files
void cleanupTestFiles() {
    remove("test1.txt");
    remove("test2.bin");
    remove("empty.txt");
    remove("test3.bin");
}

int main() {
    setupTestFiles();

    std::string testFiles[] = {
        "test1.txt",        // Case 1: Simple text file
        "test2.bin",        // Case 2: Simple binary file
        "empty.txt",        // Case 3: Empty file
        "nonexistent.file", // Case 4: File that does not exist
        "test3.bin"         // Case 5: File with a null byte
    };

    for (int i = 0; i < 5; ++i) {
        const std::string& file = testFiles[i];
        std::cout << "--- Test Case " << (i + 1) << ": " << file << " ---" << std::endl;
        std::vector<char> content = img(file);

        if (!content.empty()) {
            std::cout << "Success: Read " << content.size() << " bytes." << std::endl;
            print_content(content);
        } else {
             // Differentiate between empty file and read failure
            std::ifstream test_existence(file);
            if (test_existence.good()) {
                 std::cout << "Success: Read 0 bytes (file is empty)." << std::endl;
            } else {
                 std::cout << "Failure: File could not be read (or does not exist)." << std::endl;
            }
        }
        std::cout << std::endl;
    }

    cleanupTestFiles();
    return 0;
}