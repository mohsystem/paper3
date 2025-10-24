
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <limits>

// Validates that the filename contains only safe characters and is within a reasonable length
bool isValidFilename(const std::string& filename) {
    // Check length: must be non-empty and not exceed reasonable limit
    if (filename.empty() || filename.length() > 255) {
        return false;
    }
    
    // Reject paths with directory traversal attempts or absolute paths
    if (filename.find("..") != std::string::npos || 
        filename.find('/') != std::string::npos ||
        filename.find('\\\\') != std::string::npos ||\n        filename[0] == '.' ||\n        (filename.length() > 1 && filename[1] == ':')) { // Windows drive letter\n        return false;\n    }\n    \n    // Reject shell metacharacters and control characters\n    const char* dangerous = "|&;$`\\n<>()[]{}*?!~'\\"";
    for (char c : filename) {
        if (std::strchr(dangerous, c) != nullptr || c < 32 || c == 127) {
            return false;
        }
    }
    
    return true;
}

// Safely reads and displays file contents without using system commands
// This avoids command injection vulnerabilities entirely
bool displayFileContents(const std::string& filename) {
    // Open file in binary mode for safe reading
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file '" << filename << "'" << std::endl;
        return false;
    }
    
    // Read and display contents using safe C++ streams
    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        std::streamsize bytesRead = file.gcount();
        // Write to stdout safely with bounds checking
        std::cout.write(buffer.data(), bytesRead);
    }
    
    file.close();
    return true;
}

int main() {
    // Test cases
    std::vector<std::string> testCases = {
        "test1.txt",
        "test2.txt",
        "test3.txt",
        "test4.txt",
        "test5.txt"
    };
    
    // Create test files for demonstration
    for (size_t i = 0; i < testCases.size(); i++) {
        std::ofstream testFile(testCases[i]);
        if (testFile.is_open()) {
            testFile << "Test content for file " << (i + 1) << std::endl;
            testFile.close();
        }
    }
    
    std::cout << "=== Running Test Cases ===" << std::endl;
    for (const auto& testFile : testCases) {
        std::cout << "\\n--- Testing file: " << testFile << " ---" << std::endl;
        
        // Validate filename before processing
        if (!isValidFilename(testFile)) {
            std::cerr << "Error: Invalid filename '" << testFile << "'" << std::endl;
            continue;
        }
        
        // Display contents safely without system commands
        if (!displayFileContents(testFile)) {
            std::cerr << "Failed to display file contents" << std::endl;
        }
    }
    
    std::cout << "\\n\\n=== Interactive Mode ===" << std::endl;
    std::cout << "Enter filename to display (or 'quit' to exit): ";
    
    std::string filename;
    while (std::getline(std::cin, filename)) {
        if (filename == "quit") {
            break;
        }
        
        // Validate input filename
        if (!isValidFilename(filename)) {
            std::cerr << "Error: Invalid filename. Filenames must:" << std::endl;
            std::cerr << "  - Be 1-255 characters" << std::endl;
            std::cerr << "  - Not contain path separators or '..'" << std::endl;
            std::cerr << "  - Not contain shell metacharacters" << std::endl;
            std::cout << "\\nEnter filename to display (or 'quit' to exit): ";
            continue;
        }
        
        // Display file contents using safe file I/O
        displayFileContents(filename);
        
        std::cout << "\\nEnter filename to display (or 'quit' to exit): ";
    }
    
    return 0;
}
