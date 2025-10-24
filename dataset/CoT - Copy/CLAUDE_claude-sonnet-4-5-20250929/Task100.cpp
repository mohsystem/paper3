
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB

void searchFileWithRegex(const std::string& regexPattern, const std::string& fileName) {
    // Validate inputs
    if (regexPattern.empty()) {
        std::cerr << "Error: Regex pattern cannot be empty" << std::endl;
        return;
    }
    
    if (fileName.empty()) {
        std::cerr << "Error: File name cannot be empty" << std::endl;
        return;
    }
    
    try {
        // Validate file path
        fs::path filePath = fs::path(fileName).lexically_normal();
        
        // Check if file exists and is regular file
        if (!fs::exists(filePath)) {
            std::cerr << "Error: File does not exist: " << fileName << std::endl;
            return;
        }
        
        if (!fs::is_regular_file(filePath)) {
            std::cerr << "Error: Path is not a regular file: " << fileName << std::endl;
            return;
        }
        
        // Check file size
        if (fs::file_size(filePath) > MAX_FILE_SIZE) {
            std::cerr << "Error: File too large (max 10MB)" << std::endl;
            return;
        }
        
        // Compile regex pattern
        std::regex pattern;
        try {
            pattern = std::regex(regexPattern);
        } catch (const std::regex_error& e) {
            std::cerr << "Error: Invalid regex pattern: " << e.what() << std::endl;
            return;
        }
        
        // Read and search file content
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file: " << fileName << std::endl;
            return;
        }
        
        std::string line;
        int lineNumber = 0;
        bool found = false;
        
        while (std::getline(file, line)) {
            lineNumber++;
            std::smatch matches;
            std::string::const_iterator searchStart(line.cbegin());
            
            while (std::regex_search(searchStart, line.cend(), matches, pattern)) {
                found = true;
                std::cout << "Line " << lineNumber << ": " << line << std::endl;
                std::cout << "  Match: \\"" << matches[0] << "\\" at position " 
                         << (matches.position(0) + (searchStart - line.cbegin())) << std::endl;
                searchStart = matches.suffix().first;
            }
        }
        
        file.close();
        
        if (!found) {
            std::cout << "No matches found." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}

void cleanupTestFiles() {
    std::remove("test1.txt");
    std::remove("test2.txt");
    std::remove("test3.txt");
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        searchFileWithRegex(argv[1], argv[2]);
    } else {
        // Test cases
        std::cout << "=== Test Case 1: Search for email pattern ===" << std::endl;
        createTestFile("test1.txt", "Contact: john@example.com\\nEmail: jane@test.org");
        searchFileWithRegex(R"(\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b)", "test1.txt");
        
        std::cout << "\\n=== Test Case 2: Search for phone numbers ===" << std::endl;
        createTestFile("test2.txt", "Call: 123-456-7890\\nPhone: 555-0123");
        searchFileWithRegex(R"(\\d{3}-\\d{3}-\\d{4})", "test2.txt");
        
        std::cout << "\\n=== Test Case 3: No matches ===" << std::endl;
        createTestFile("test3.txt", "No numbers here at all!");
        searchFileWithRegex(R"(\\d+)", "test3.txt");
        
        std::cout << "\\n=== Test Case 4: Invalid regex ===" << std::endl;
        searchFileWithRegex("[invalid(", "test1.txt");
        
        std::cout << "\\n=== Test Case 5: Non-existent file ===" << std::endl;
        searchFileWithRegex("test", "nonexistent.txt");
        
        cleanupTestFiles();
    }
    
    return 0;
}
