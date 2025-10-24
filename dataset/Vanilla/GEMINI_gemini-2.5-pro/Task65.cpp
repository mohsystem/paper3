#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem> // Requires C++17. For older standards, use platform-specific APIs.

/**
 * @brief Retrieves a filename, validates it, and returns the file's content.
 * 
 * @param filename The path to the file.
 * @return A string containing the file content or an error message.
 */
std::string readFileContent(const std::string& filename) {
    if (filename.empty() || filename.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
        return "Error: Filename cannot be null or empty.";
    }

    std::error_code ec;
    if (!std::filesystem::exists(filename, ec)) {
        return "Error: File does not exist: " + filename;
    }
    if (ec) {
        return "Error checking existence: " + ec.message();
    }
    
    if (std::filesystem::is_directory(filename, ec)) {
        return "Error: Path points to a directory, not a file: " + filename;
    }
    if (ec) {
        return "Error checking path type: " + ec.message();
    }

    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        return "Error: Cannot open file for reading (check permissions): " + filename;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    file.close();
    return content;
}

// Helper to create a test file
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    }
}

void runTests() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Valid and existing file
    std::cout << "\n--- Test Case 1: Valid File ---" << std::endl;
    std::string validFile = "test_valid.txt";
    createTestFile(validFile, "Hello C++!\nThis is a test file.");
    std::cout << "Result: \n" << readFileContent(validFile) << std::endl;
    std::filesystem::remove(validFile);

    // Test Case 2: Non-existent file
    std::cout << "\n--- Test Case 2: Non-existent File ---" << std::endl;
    std::cout << "Result: " << readFileContent("nonexistent.txt") << std::endl;

    // Test Case 3: Filename is a directory
    std::cout << "\n--- Test Case 3: Path is a Directory ---" << std::endl;
    std::string dirName = "test_dir";
    std::filesystem::create_directory(dirName);
    std::cout << "Result: " << readFileContent(dirName) << std::endl;
    std::filesystem::remove(dirName);

    // Test Case 4: Empty filename
    std::cout << "\n--- Test Case 4: Empty Filename ---" << std::endl;
    std::cout << "Result: " << readFileContent("") << std::endl;

    // Test Case 5: Whitespace filename
    std::cout << "\n--- Test Case 5: Whitespace Filename ---" << std::endl;
    std::cout << "Result: " << readFileContent("   ") << std::endl;
    
    std::cout << "\n--- All tests completed ---" << std::endl;
}

int main(int argc, char* argv[]) {
    // Example with command-line arguments
    if (argc > 1) {
        std::cout << "--- Reading from command line argument ---" << std::endl;
        std::string filename = argv[1];
        std::cout << "Reading file: " << filename << std::endl;
        std::string content = readFileContent(filename);
        std::cout << "Content:\n" << content << std::endl;
        std::cout << "----------------------------------------\n" << std::endl;
    } else {
        std::cout << "Usage: ./your_program_name <filename>\n" << std::endl;
    }

    runTests();

    return 0;
}