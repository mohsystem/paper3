#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>

/**
 * @brief Securely reads the content of a file after validating the filename.
 *
 * The validation uses a whitelist regex to ensure the filename only contains safe characters
 * and does not contain path separators or traversal sequences, restricting access to the 
 * current directory.
 *
 * @param filename The name of the file to read.
 * @return A string containing the file content or an error message.
 */
std::string getFileContent(const std::string& filename) {
    if (filename.empty()) {
        return "Error: Filename cannot be empty.";
    }

    // Security Validation: Whitelist approach.
    // Allow only alphanumeric characters, dots, underscores, and hyphens.
    const std::regex whitelist_pattern("^[a-zA-Z0-9._-]+$");
    if (!std::regex_match(filename, whitelist_pattern)) {
        return "Error: Invalid filename format. Path separators and special characters are not allowed.";
    }

    // Explicitly check for ".." which can be used in tricky ways.
    if (filename.find("..") != std::string::npos) {
        return "Error: Path traversal sequences ('..') are not allowed.";
    }

    std::ifstream file_stream(filename, std::ios::in | std::ios::binary);

    if (!file_stream) {
        // We don't distinguish between "not found" and "permission denied" to avoid leaking 
        // information about the filesystem structure.
        return "Error: Could not open file. It may not exist or you may not have permission to read it.";
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    
    if (file_stream.fail() && !file_stream.eof()) {
        file_stream.close();
        return "Error: An I/O error occurred while reading the file.";
    }
    
    file_stream.close();
    return buffer.str();
}

void setupTestFiles() {
    std::ofstream test1("test1.txt");
    if (test1.is_open()) {
        test1 << "This is a secure file." << std::endl;
        test1.close();
    } else {
        std::cerr << "Warning: Could not create test file 'test1.txt'." << std::endl;
    }
}

int main() {
    setupTestFiles();

    std::vector<std::string> testCases = {
        "test1.txt",              // 1. Valid file
        "non_existent.txt",       // 2. Non-existent file
        "../other_dir/secret.txt",// 3. Path traversal attempt
        "safe_dir/test2.txt",     // 4. Filename with directory separator
        "/etc/passwd"             // 5. Absolute path attempt
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": \"" << testCases[i] << "\" ---" << std::endl;
        std::string result = getFileContent(testCases[i]);
        std::cout << "Result:\n" << result << std::endl << std::endl;
    }

    return 0;
}