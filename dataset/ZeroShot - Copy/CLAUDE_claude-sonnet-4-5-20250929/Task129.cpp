
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <regex>
#include <filesystem>
#include <cstring>

class Task129 {
private:
    static const std::string BASE_DIRECTORY;
    static const std::set<std::string> ALLOWED_EXTENSIONS;
    static const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    
    static std::string getFileExtension(const std::string& filename) {
        size_t lastDot = filename.find_last_of('.');
        if (lastDot != std::string::npos && lastDot < filename.length() - 1) {
            return filename.substr(lastDot);
        }
        return "";
    }
    
    static std::string toLower(const std::string& str) {
        std::string result = str;
        for (char& c : result) {
            c = std::tolower(c);
        }
        return result;
    }
    
public:
    static std::string retrieveFile(const std::string& fileName) {
        try {
            // Input validation
            if (fileName.empty() || fileName.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
                return "Error: File name cannot be empty";
            }
            
            std::string trimmedName = fileName;
            trimmedName.erase(0, trimmedName.find_first_not_of(" \\t\\n\\r"));
            trimmedName.erase(trimmedName.find_last_not_of(" \\t\\n\\r") + 1);
            
            // Check for safe filename pattern
            std::regex safePattern("^[a-zA-Z0-9_\\\\-\\\\.]+$");
            if (!std::regex_match(trimmedName, safePattern)) {
                return "Error: Invalid file name format. Only alphanumeric, dash, underscore and dot allowed";
            }
            
            // Check file extension
            std::string extension = toLower(getFileExtension(trimmedName));
            if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {
                return "Error: File type not allowed";
            }
            
            // Prevent path traversal attacks
            std::filesystem::path basePath = std::filesystem::absolute(BASE_DIRECTORY);
            std::filesystem::path filePath = std::filesystem::absolute(basePath / trimmedName);
            
            std::string baseStr = basePath.string();
            std::string fileStr = filePath.string();
            
            if (fileStr.substr(0, baseStr.length()) != baseStr) {
                return "Error: Path traversal detected. Access denied";
            }
            
            // Check if file exists
            if (!std::filesystem::exists(filePath)) {
                return "Error: File not found";
            }
            
            // Check if it's a file (not directory)
            if (!std::filesystem::is_regular_file(filePath)) {
                return "Error: Invalid file type";
            }
            
            // Check file size
            if (std::filesystem::file_size(filePath) > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size";
            }
            
            // Read file content
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return "Error: Unable to open file";
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            return "Success: File retrieved\\nContent:\\n" + buffer.str();
            
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
    
    static void setupTestEnvironment() {
        try {
            std::filesystem::create_directories(BASE_DIRECTORY);
            
            // Create test files
            std::ofstream file1(BASE_DIRECTORY + "test1.txt");
            file1 << "This is a test file 1";
            file1.close();
            
            std::ofstream file2(BASE_DIRECTORY + "test2.json");
            file2 << "{\\"name\\": \\"test\\", \\"value\\": 123}";
            file2.close();
            
            std::ofstream file3(BASE_DIRECTORY + "test3.csv");
            file3 << "id,name,age\\n1,John,30\\n2,Jane,25";
            file3.close();
        } catch (const std::exception& e) {
            std::cerr << "Setup error: " << e.what() << std::endl;
        }
    }
};

const std::string Task129::BASE_DIRECTORY = "./secure_files/";
const std::set<std::string> Task129::ALLOWED_EXTENSIONS = {".txt", ".json", ".xml", ".csv", ".log"};

int main() {
    Task129::setupTestEnvironment();
    
    std::cout << "=== Secure File Retrieval System ===\\n\\n";
    
    // Test case 1: Valid file retrieval
    std::cout << "Test 1: Valid file retrieval\\n";
    std::cout << Task129::retrieveFile("test1.txt") << "\\n---\\n\\n";
    
    // Test case 2: Another valid file
    std::cout << "Test 2: Valid JSON file\\n";
    std::cout << Task129::retrieveFile("test2.json") << "\\n---\\n\\n";
    
    // Test case 3: Path traversal attempt
    std::cout << "Test 3: Path traversal attempt\\n";
    std::cout << Task129::retrieveFile("../../../etc/passwd") << "\\n---\\n\\n";
    
    // Test case 4: Invalid file extension
    std::cout << "Test 4: Invalid file extension\\n";
    std::cout << Task129::retrieveFile("test.exe") << "\\n---\\n\\n";
    
    // Test case 5: Non-existent file
    std::cout << "Test 5: Non-existent file\\n";
    std::cout << Task129::retrieveFile("nonexistent.txt") << "\\n---\\n\\n";
    
    return 0;
}
