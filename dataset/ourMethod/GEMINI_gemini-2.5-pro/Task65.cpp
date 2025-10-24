#include <iostream>
#include <string>
#include <fstream>
#include <optional>
#include <filesystem>
#include <regex>

// Rule #1, #5: Validate filename format.
// Whitelist of allowed characters: alphanumeric, dot, underscore, hyphen. Length 1-255.
bool isFilenameValid(const std::string& filename) {
    if (filename.empty() || filename.length() > 255) {
        return false;
    }
    static const std::regex pattern("^[a-zA-Z0-9._-]+$");
    return std::regex_match(filename, pattern);
}

/**
 * Validates a filename and reads its content if it is a regular, accessible file.
 *
 * @param filename The name of the file to read, must be in the current directory.
 * @return An optional containing the file content as a string, or an empty optional on error.
 */
std::optional<std::string> readFileContent(const std::string& filename) {
    if (!isFilenameValid(filename)) {
        std::cerr << "Error: Invalid filename '" << filename << "'. Only alphanumeric, '.', '_', '-' are allowed. No path separators." << std::endl;
        return std::nullopt;
    }

    try {
        const std::filesystem::path path(filename);

        // Rule #5, #6: Check for existence and that it's a regular file.
        // std::filesystem::is_regular_file checks for existence and that it's not a directory/symlink.
        if (!std::filesystem::is_regular_file(path)) {
            if (!std::filesystem::exists(path)) {
                std::cerr << "Error: File does not exist: " << filename << std::endl;
            } else {
                std::cerr << "Error: Path is not a regular file: " << filename << std::endl;
            }
            return std::nullopt;
        }

        // Rule #7: Handle potential exceptions during file I/O.
        // RAII is used for file handle management via ifstream.
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file (check permissions): " << filename << std::endl;
            return std::nullopt;
        }

        // Rule #3: Read file contents without buffer overflow
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        file.close();
        return content;
        
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

void testSuccess(const std::string& testName, const std::string& filename) {
    std::cout << "\n" << testName << " (" << filename << "):" << std::endl;
    auto content = readFileContent(filename);
    if (content) {
        std::cout << "Success. Content is " << (content->empty() ? "empty." : "not empty.") << std::endl;
    } else {
        std::cerr << "Failed unexpectedly." << std::endl;
    }
}

void testFailure(const std::string& testName, const std::string& filename) {
    std::cout << "\n" << testName << " (" << filename << "):" << std::endl;
    auto content = readFileContent(filename);
    if (content) {
        std::cerr << "Failed. Read content unexpectedly." << std::endl;
    } else {
        std::cout << "Success. Function returned empty as expected." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // --- Test Cases ---
    std::cout << "--- Running Test Cases ---" << std::endl;
    const std::string validFile = "test1.txt";
    const std::string emptyFile = "empty.txt";
    const std::string nonExistentFile = "no_such_file.txt";
    const std::string directoryName = "test_dir";
    const std::string invalidFilename = "../test1.txt";

    // Setup
    std::filesystem::create_directory(directoryName);
    { std::ofstream ofs(validFile); ofs << "Hello, C++ World!\n"; }
    { std::ofstream ofs(emptyFile); }

    // Run tests
    testSuccess("1. Testing with a valid file", validFile);
    testFailure("2. Testing with a non-existent file", nonExistentFile);
    testFailure("3. Testing with an invalid filename", invalidFilename);
    testFailure("4. Testing with a directory", directoryName);
    testSuccess("5. Testing with an empty file", emptyFile);
    
    // Cleanup
    std::filesystem::remove(validFile);
    std::filesystem::remove(emptyFile);
    std::filesystem::remove(directoryName);

    std::cout << "\n--- End of Test Cases ---" << std::endl;

    // --- Main Program Logic ---
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::cout << "\nAttempting to read from command line argument: " << filename << std::endl;
    auto content = readFileContent(filename);

    if (content) {
        std::cout << "--- File Content ---" << std::endl;
        std::cout << *content;
        std::cout << "--- End of Content ---" << std::endl;
    } else {
        std::cerr << "Could not read file." << std::endl;
        return 1;
    }

    return 0;
}