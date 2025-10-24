
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <limits>
#include <cstring>

// Security: Validate file path to prevent directory traversal
// Returns true if path is safe (no .., absolute paths within expected scope)
bool isValidFilePath(const std::string& filepath) {
    // Reject empty paths
    if (filepath.empty() || filepath.length() > 4096) {
        return false;
    }
    
    // Reject paths with directory traversal attempts
    if (filepath.find("..") != std::string::npos) {
        return false;
    }
    
    // Reject null bytes (path truncation attack)
    if (filepath.find('\\0') != std::string::npos) {
        return false;
    }
    
    return true;
}

// Security: Safe file reading with bounds checking
// Reads file line by line, prevents memory exhaustion
std::vector<std::string> readFileSafely(const std::string& filepath) {
    std::vector<std::string> lines;
    
    // Security: Validate file path before opening
    if (!isValidFilePath(filepath)) {
        std::cerr << "Invalid file path" << std::endl;
        return lines;
    }
    
    // Security: Open file in read-only mode
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return lines;
    }
    
    std::string line;
    const size_t MAX_LINE_LENGTH = 1048576; // 1MB per line limit
    const size_t MAX_LINES = 1000000; // Limit total lines to prevent memory exhaustion
    
    // Security: Read with bounds checking to prevent memory exhaustion
    while (std::getline(file, line) && lines.size() < MAX_LINES) {
        // Security: Validate line length to prevent memory exhaustion
        if (line.length() > MAX_LINE_LENGTH) {
            std::cerr << "Line too long, skipping" << std::endl;
            continue;
        }
        lines.push_back(line);
    }
    
    file.close();
    return lines;
}

// Match lines that start with numbers and end with a dot
std::vector<std::string> matchLines(const std::vector<std::string>& lines) {
    std::vector<std::string> matched;
    
    // Security: Use safe regex with complexity limits
    // Pattern: starts with one or more digits, ends with a dot
    try {
        std::regex pattern("^\\\\d+.*\\\\.$");
        
        for (const auto& line : lines) {
            // Security: Protect against ReDoS by limiting input size (already done in read)
            if (std::regex_match(line, pattern)) {
                matched.push_back(line);
            }
        }
    } catch (const std::regex_error& e) {
        std::cerr << "Regex error: " << e.what() << std::endl;
    }
    
    return matched;
}

int main() {
    // Test case 1: Basic file with matching lines
    {
        const char* filename = "test1.txt";
        std::ofstream out(filename);
        if (out.is_open()) {
            out << "123 This line starts with numbers and ends with a dot.\\n";
            out << "This line does not start with numbers.\\n";
            out << "456 Another matching line.\\n";
            out << "789 No dot at the end\\n";
            out << "0 Single digit.\\n";
            out.close();
            
            auto lines = readFileSafely(filename);
            auto matched = matchLines(lines);
            std::cout << "Test 1 - Matched lines:" << std::endl;
            for (const auto& line : matched) {
                std::cout << "  " << line << std::endl;
            }
        }
    }
    
    // Test case 2: Empty file
    {
        const char* filename = "test2.txt";
        std::ofstream out(filename);
        out.close();
        
        auto lines = readFileSafely(filename);
        auto matched = matchLines(lines);
        std::cout << "Test 2 - Matched lines: " << matched.size() << std::endl;
    }
    
    // Test case 3: No matching lines
    {
        const char* filename = "test3.txt";
        std::ofstream out(filename);
        if (out.is_open()) {
            out << "No numbers at start.\\n";
            out << "Also no numbers here\\n";
            out.close();
            
            auto lines = readFileSafely(filename);
            auto matched = matchLines(lines);
            std::cout << "Test 3 - Matched lines: " << matched.size() << std::endl;
        }
    }
    
    // Test case 4: Invalid file path (directory traversal attempt)
    {
        auto lines = readFileSafely("../etc/passwd");
        std::cout << "Test 4 - Lines read (should be 0): " << lines.size() << std::endl;
    }
    
    // Test case 5: Multiple digits and special characters
    {
        const char* filename = "test5.txt";
        std::ofstream out(filename);
        if (out.is_open()) {
            out << "999 Line with special chars !@#$%.\\n";
            out << "42.\\n";
            out << "100 Multiword sentence here.\\n";
            out.close();
            
            auto lines = readFileSafely(filename);
            auto matched = matchLines(lines);
            std::cout << "Test 5 - Matched lines:" << std::endl;
            for (const auto& line : matched) {
                std::cout << "  " << line << std::endl;
            }
        }
    }
    
    return 0;
}
