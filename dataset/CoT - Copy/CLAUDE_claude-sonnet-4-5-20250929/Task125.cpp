
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>

class Task125 {
private:
    static std::map<std::string, std::map<std::string, std::string>> USER_RESOURCES;
    static std::regex VALID_USERNAME_PATTERN;
    static std::regex VALID_RESOURCE_PATTERN;
    
    static void initializeResources() {
        static bool initialized = false;
        if (!initialized) {
            USER_RESOURCES["user1"]["document1"] = "Content of Document 1 for User1";
            USER_RESOURCES["user1"]["report2"] = "Annual Report Data for User1";
            
            USER_RESOURCES["user2"]["file3"] = "Private File Content for User2";
            USER_RESOURCES["user2"]["data4"] = "Database Export for User2";
            
            USER_RESOURCES["admin"]["config"] = "System Configuration Data";
            USER_RESOURCES["admin"]["logs"] = "System Logs and Monitoring";
            
            initialized = true;
        }
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, (last - first + 1));
    }

public:
    static std::string accessResource(const std::string& username, const std::string& resourceName) {
        initializeResources();
        
        // Input validation - check for empty strings
        if (username.empty() || resourceName.empty()) {
            return "Error: Invalid input - empty values not allowed";
        }
        
        // Sanitize and validate username
        std::string cleanUsername = trim(username);
        if (!std::regex_match(cleanUsername, VALID_USERNAME_PATTERN)) {
            return "Error: Invalid username format";
        }
        
        // Sanitize and validate resource name
        std::string cleanResourceName = trim(resourceName);
        if (!std::regex_match(cleanResourceName, VALID_RESOURCE_PATTERN)) {
            return "Error: Invalid resource name format";
        }
        
        // Check if user exists
        if (USER_RESOURCES.find(cleanUsername) == USER_RESOURCES.end()) {
            return "Error: User not found";
        }
        
        // Check if user has access to the resource
        auto& userResources = USER_RESOURCES[cleanUsername];
        if (userResources.find(cleanResourceName) == userResources.end()) {
            return "Error: Access denied - Resource not found or unauthorized";
        }
        
        // Return the resource content
        return "Success: " + userResources[cleanResourceName];
    }
};

// Static member initialization
std::map<std::string, std::map<std::string, std::string>> Task125::USER_RESOURCES;
std::regex Task125::VALID_USERNAME_PATTERN("^[a-zA-Z0-9_]{3,20}$");
std::regex Task125::VALID_RESOURCE_PATTERN("^[a-zA-Z0-9_]{1,50}$");

int main() {
    std::cout << "=== Secure Resource Access Control System ===\\n\\n";
    
    // Test Case 1: Valid access
    std::cout << "Test 1 - Valid Access:\\n";
    std::cout << Task125::accessResource("user1", "document1") << "\\n\\n";
    
    // Test Case 2: Valid access different user
    std::cout << "Test 2 - Valid Access (User2):\\n";
    std::cout << Task125::accessResource("user2", "file3") << "\\n\\n";
    
    // Test Case 3: Unauthorized access attempt
    std::cout << "Test 3 - Unauthorized Access:\\n";
    std::cout << Task125::accessResource("user1", "file3") << "\\n\\n";
    
    // Test Case 4: Path traversal attempt
    std::cout << "Test 4 - Path Traversal Attack Attempt:\\n";
    std::cout << Task125::accessResource("user1", "../admin/config") << "\\n\\n";
    
    // Test Case 5: Empty input handling
    std::cout << "Test 5 - Empty Input:\\n";
    std::cout << Task125::accessResource("", "document1") << "\\n\\n";
    
    return 0;
}
