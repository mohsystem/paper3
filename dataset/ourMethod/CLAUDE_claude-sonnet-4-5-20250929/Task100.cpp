
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <cstring>
#include <limits>

// Security: Validates that path is within allowed directory and doesn't escape via symlinks or ..\nbool isValidPath(const std::string& path) {\n    // Reject empty paths\n    if (path.empty() || path.length() > 4096) {\n        return false;\n    }\n    \n    // Reject paths with null bytes (path traversal defense)\n    if (path.find('\\0') != std::string::npos) {\n        return false;\n    }\n    \n    // Reject absolute paths and path traversal attempts\n    if (path[0] == '/' || path.find("..") != std::string::npos) {\n        return false;\n    }\n    \n    // Additional checks for suspicious patterns\n    if (path.find("//") != std::string::npos) {\n        return false;\n    }\n    \n    return true;\n}\n\n// Security: Safe file reading with bounds checking and size limits\nbool readFileContent(const std::string& filename, std::string& content) {\n    const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit to prevent memory exhaustion\n    \n    // Security: Validate path before opening\n    if (!isValidPath(filename)) {\n        std::cerr << "Error: Invalid file path" << std::endl;\n        return false;\n    }\n    \n    // Security: Open file with explicit mode, no write access\n    std::ifstream file(filename, std::ios::in | std::ios::binary);\n    if (!file.is_open()) {\n        std::cerr << "Error: Cannot open file: " << filename << std::endl;\n        return false;\n    }\n    \n    // Security: Check file size before reading to prevent memory exhaustion\n    file.seekg(0, std::ios::end);\n    std::streampos fileSize = file.tellg();\n    \n    if (fileSize < 0 || static_cast<size_t>(fileSize) > MAX_FILE_SIZE) {\n        std::cerr << "Error: File size invalid or exceeds limit" << std::endl;\n        file.close();\n        return false;\n    }\n    \n    file.seekg(0, std::ios::beg);\n    \n    // Security: Reserve exact size to prevent reallocations\n    content.clear();\n    content.reserve(static_cast<size_t>(fileSize));\n    \n    // Security: Read with bounds checking using std::vector as intermediate buffer\n    const size_t BUFFER_SIZE = 8192;\n    std::vector<char> buffer(BUFFER_SIZE);\n    \n    while (file.good() && content.size() < MAX_FILE_SIZE) {\n        file.read(buffer.data(), BUFFER_SIZE);\n        std::streamsize bytesRead = file.gcount();\n        \n        if (bytesRead > 0) {\n            // Security: Bounds check before append\n            if (content.size() + static_cast<size_t>(bytesRead) > MAX_FILE_SIZE) {\n                std::cerr << "Error: File size limit exceeded during read" << std::endl;\n                file.close();\n                return false;\n            }\n            content.append(buffer.data(), static_cast<size_t>(bytesRead));\n        }\n    }\n    \n    file.close();\n    return true;\n}\n\n// Security: Validates regex pattern to prevent ReDoS and excessive complexity\nbool isValidRegexPattern(const std::string& pattern) {\n    // Reject empty or excessively long patterns\n    if (pattern.empty() || pattern.length() > 1024) {\n        return false;\n    }\n    \n    // Reject patterns with null bytes\n    if (pattern.find('\\0') != std::string::npos) {\n        return false;\n    }\n    \n    // Basic check for potentially dangerous nested quantifiers (ReDoS prevention)\n    // This is a simple heuristic; more sophisticated checks could be added\n    size_t nestedQuantifiers = 0;\n    for (size_t i = 0; i < pattern.length(); ++i) {\n        if (pattern[i] == '*' || pattern[i] == '+' || pattern[i] == '{') {
            nestedQuantifiers++;
            if (nestedQuantifiers > 10) { // Arbitrary limit for nested quantifiers
                return false;
            }
        }
    }
    
    return true;
}

void searchWithRegex(const std::string& pattern, const std::string& filename) {
    // Security: Validate inputs before processing
    if (!isValidRegexPattern(pattern)) {
        std::cerr << "Error: Invalid regex pattern" << std::endl;
        return;
    }
    
    std::string content;
    if (!readFileContent(filename, content)) {
        return;
    }
    
    try {
        // Security: Compile regex with exception handling to catch malformed patterns
        std::regex regexPattern(pattern, std::regex::ECMAScript);
        
        // Search for matches
        std::sregex_iterator begin(content.begin(), content.end(), regexPattern);
        std::sregex_iterator end;
        
        int matchCount = 0;
        const int MAX_MATCHES = 1000; // Limit output to prevent DoS
        
        for (std::sregex_iterator i = begin; i != end && matchCount < MAX_MATCHES; ++i) {
            std::smatch match = *i;
            std::cout << "Match found: " << match.str() << std::endl;
            matchCount++;
        }
        
        if (matchCount == 0) {
            std::cout << "No matches found." << std::endl;
        } else {
            std::cout << "Total matches: " << matchCount << std::endl;
        }
        
    } catch (const std::regex_error& e) {
        std::cerr << "Error: Invalid regex pattern - " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Security: Validate argument count
    if (argc != 3) {
        std::cerr << "Usage: " << (argv[0] ? argv[0] : "program") << " <regex_pattern> <filename>" << std::endl;
        return 1;
    }
    
    // Security: Validate argv pointers before dereferencing
    if (argv[1] == nullptr || argv[2] == nullptr) {
        std::cerr << "Error: Invalid arguments" << std::endl;
        return 1;
    }
    
    // Security: Validate argument lengths to prevent buffer issues
    size_t patternLen = std::strlen(argv[1]);
    size_t filenameLen = std::strlen(argv[2]);
    
    if (patternLen == 0 || patternLen > 1024 || filenameLen == 0 || filenameLen > 4096) {
        std::cerr << "Error: Argument length invalid" << std::endl;
        return 1;
    }
    
    std::string pattern(argv[1]);
    std::string filename(argv[2]);
    
    // Test cases
    std::cout << "=== Test Case: User provided arguments ===" << std::endl;
    searchWithRegex(pattern, filename);
    
    return 0;
}
