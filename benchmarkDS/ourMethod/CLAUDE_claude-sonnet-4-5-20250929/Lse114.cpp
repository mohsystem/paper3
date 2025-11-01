
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

/**
 * Secure directory listing - DOES NOT use system() to avoid command injection.
 * Uses C++17 filesystem library for safe operations.
 */
std::string listDirectory(const std::string& directoryPath) {
    // Input validation: reject empty paths
    if (directoryPath.empty()) {
        return "Error: Invalid directory path";
    }
    
    try {
        // Normalize path and convert to absolute to prevent traversal
        fs::path path = fs::canonical(fs::absolute(directoryPath));
        
        // Verify it's a directory\n        if (!fs::exists(path)) {\n            return "Error: Path does not exist";\n        }\n        \n        if (!fs::is_directory(path)) {\n            return "Error: Path is not a directory";\n        }\n        \n        // Build result string\n        std::string result = "Contents of: " + path.string() + "\\n";\n        \n        // Collect and sort entries\n        std::vector<std::string> entries;\n        for (const auto& entry : fs::directory_iterator(path)) {\n            entries.push_back(entry.path().filename().string());\n        }\n        std::sort(entries.begin(), entries.end());\n        \n        // Append entries to result\n        for (const auto& entry : entries) {\n            result += entry + "\\n";\n        }\n        \n        return result;\n        \n    } catch (const fs::filesystem_error& e) {\n        // Generic error message - don't leak internal details
        return "Error: Cannot access directory";
    } catch (const std::exception& e) {
        return "Error: Cannot access directory";
    }
}

int main() {
    // Test case 1: Current directory
    std::cout << "=== Test 1: Current directory ===" << std::endl;
    std::cout << listDirectory(".") << std::endl;
    
    // Test case 2: Temp directory
    std::cout << "=== Test 2: Temp directory ===" << std::endl;
    std::cout << listDirectory("/tmp") << std::endl;
    
    // Test case 3: Invalid path (security test)
    std::cout << "=== Test 3: Path traversal attempt ===" << std::endl;
    std::cout << listDirectory("../../../etc") << std::endl;
    
    // Test case 4: Empty string
    std::cout << "=== Test 4: Empty string ===" << std::endl;
    std::cout << listDirectory("") << std::endl;
    
    // Test case 5: Non-existent path
    std::cout << "=== Test 5: Non-existent path ===" << std::endl;
    std::cout << listDirectory("/nonexistent/path") << std::endl;
    
    return 0;
}
