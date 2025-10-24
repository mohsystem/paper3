#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio> // For std::remove

/**
 * Encrypts or decrypts a file using a simple XOR cipher with a given key.
 * The operation is symmetric; applying it twice restores the original file.
 *
 * @param key The secret key for encryption/decryption.
 * @param inputFilePath Path to the input file.
 * @param outputFilePath Path to the output file.
 */
void processFile(const std::string& key, const std::string& inputFilePath, const std::string& outputFilePath) {
    if (key.empty()) {
        std::cerr << "Error: Encryption key cannot be empty." << std::endl;
        return;
    }

    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error: Cannot open input file " << inputFilePath << std::endl;
        return;
    }

    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Cannot open output file " << outputFilePath << std::endl;
        inputFile.close();
        return;
    }

    char c;
    size_t keyIndex = 0;
    // Read one byte at a time from the input file
    while (inputFile.get(c)) {
        // XOR the byte with the corresponding key byte and write to the output file
        outputFile.put(c ^ key[keyIndex]);
        // Move to the next key byte, wrapping around if necessary
        keyIndex = (keyIndex + 1) % key.length();
    }

    inputFile.close();
    outputFile.close();
}

// Helper function to read a file's content into a string
std::string readFileContent(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return "";
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Helper function to run a complete test case
void runTestCase(const std::string& testId, const std::string& content, const std::string& key) {
    std::string plainFile = testId + "_plain.txt";
    std::string encryptedFile = testId + "_encrypted.bin";
    std::string decryptedFile = testId + "_decrypted.txt";

    // 1. Create the original file
    std::ofstream(plainFile, std::ios::binary) << content;
    std::cout << "\n--- Running Test Case: " << testId << " ---" << std::endl;
    std::cout << "Original content: \"" << content << "\"" << std::endl;

    // 2. Encrypt
    processFile(key, plainFile, encryptedFile);
    std::cout << "'" << plainFile << "' encrypted to '" << encryptedFile << "' with key '" << key << "'." << std::endl;

    // 3. Decrypt
    processFile(key, encryptedFile, decryptedFile);
    std::cout << "'" << encryptedFile << "' decrypted to '" << decryptedFile << "'." << std::endl;

    // 4. Verify
    std::string decryptedContent = readFileContent(decryptedFile);
    if (content == decryptedContent) {
        std::cout << "SUCCESS: Decrypted content matches original content." << std::endl;
    } else {
        std::cout << "FAILURE: Content mismatch!" << std::endl;
        std::cout << "Decrypted content: \"" << decryptedContent << "\"" << std::endl;
    }

    // 5. Cleanup
    std::remove(plainFile.c_str());
    std::remove(encryptedFile.c_str());
    std::remove(decryptedFile.c_str());
}

int main() {
    std::cout << "--- C++ File Encryption/Decryption Tests ---" << std::endl;

    // Test Case 1: Simple text
    runTestCase("test1", "Hello World!", "KEY1");
    
    // Test Case 2: Different key and text
    runTestCase("test2", "CPP XOR Encryption/Decryption.", "SECRET");
    
    // Test Case 3: Longer text to test key wrapping
    runTestCase("test3", "This is a longer piece of text to ensure the key repeats correctly.", "LONGERKEY");

    // Test Case 4: Text with special characters and numbers
    runTestCase("test4", "12345!@#$%^&*()_+", "NUMSYM");

    // Test Case 5: Empty file
    runTestCase("test5", "", "EMPTY");

    std::cout << "\n--- C++ Tests Completed ---" << std::endl;
    return 0;
}