
#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <algorithm>

class Task125 {
private:
    static std::unordered_set<std::string> allowedResources;
    
    static bool isValidResourceFormat(const std::string& resourceName) {
        std::regex validPattern("^[a-zA-Z0-9._-]+$");
        return std::regex_match(resourceName, validPattern);
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, last - first + 1);
    }
    
public:
    static std::string accessResource(const std::string& username, const std::string& resourceName) {
        // Input validation
        std::string trimmedUsername = trim(username);
        if (trimmedUsername.empty()) {
            return "Error: Invalid username";
        }
        
        std::string trimmedResourceName = trim(resourceName);
        if (trimmedResourceName.empty()) {
            return "Error: Invalid resource name";
        }
        
        // Validate resource name format
        if (!isValidResourceFormat(trimmedResourceName)) {
            return "Error: Invalid resource name format";
        }
        
        // Check for path traversal attempts
        if (trimmedResourceName.find("..") != std::string::npos ||
            trimmedResourceName.find("/") != std::string::npos ||
            trimmedResourceName.find("\\\\") != std::string::npos) {\n            return "Error: Path traversal attempt detected";\n        }\n        \n        // Check if resource is in whitelist\n        if (allowedResources.find(trimmedResourceName) == allowedResources.end()) {\n            return "Error: Access denied - resource not found or unauthorized";\n        }\n        \n        // Simulate resource access\n        return "Success: User '" + trimmedUsername + "' accessed resource '" + trimmedResourceName + "'";\n    }\n};\n\nstd::unordered_set<std::string> Task125::allowedResources = {\n    "document1.txt", "document2.txt", "image1.jpg", "image2.jpg", "data.csv"\n};\n\nint main() {\n    std::cout << "=== Resource Access Control System ===\
\
";\n    \n    // Test case 1: Valid access\n    std::cout << "Test 1 - Valid access:\
";\n    std::cout << Task125::accessResource("john_doe", "document1.txt") << "\
\
";\n    \n    // Test case 2: Valid access to different resource\n    std::cout << "Test 2 - Valid access to image:\
";\n    std::cout << Task125::accessResource("jane_smith", "image1.jpg") << "\
\
";\n    \n    // Test case 3: Attempt to access non-existent resource\n    std::cout << "Test 3 - Non-existent resource:\
";\n    std::cout << Task125::accessResource("user1", "secret.txt") << "\
\
";\n    \n    // Test case 4: Path traversal attempt\n    std::cout << "Test 4 - Path traversal attempt:\
";\n    std::cout << Task125::accessResource("attacker", "../../../etc/passwd") << "\
\
";\n    \n    // Test case 5: Invalid input\n    std::cout << "Test 5 - Invalid input:\
";\n    std::cout << Task125::accessResource("", "document1.txt") << "\
";\n    std::cout << Task125::accessResource("user2", "") << "\
\
";
    
    return 0;
}
