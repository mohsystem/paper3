
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <limits>

// Resource access control system with path traversal prevention
class ResourceAccessControl {
private:
    // Whitelist of allowed resource names (no path components)
    std::map<std::string, std::string> allowedResources;
    std::string baseDirectory;
    
    // Validate resource name contains no path traversal attempts
    bool isValidResourceName(const std::string& resourceName) const {
        if (resourceName.empty() || resourceName.length() > 255) {
            return false;
        }
        
        // Reject any path separators or special sequences
        if (resourceName.find('/') != std::string::npos ||
            resourceName.find('\\\\') != std::string::npos ||\n            resourceName.find("..") != std::string::npos ||\n            resourceName.find(':') != std::string::npos ||\n            resourceName.find('\\0') != std::string::npos) {\n            return false;\n        }\n        \n        // Only allow alphanumeric, underscore, hyphen, and dot\n        for (char c : resourceName) {\n            if (!std::isalnum(static_cast<unsigned char>(c)) && \n                c != '_' && c != '-' && c != '.') {\n                return false;\n            }\n        }\n        \n        // Reject names starting with dot (hidden files)\n        if (resourceName[0] == '.') {\n            return false;\n        }\n        \n        return true;\n    }\n    \npublic:\n    ResourceAccessControl() : baseDirectory("/safe/resources") {\n        // Initialize with whitelisted resources only\n        allowedResources["document1"] = "Public document about services";\n        allowedResources["document2"] = "User guide for application";\n        allowedResources["report_2024"] = "Annual report data";\n        allowedResources["config_template"] = "Configuration template";\n    }\n    \n    // Access resource by name with strict validation\n    std::string accessResource(const std::string& resourceName) {\n        // Input validation: check for valid resource name format\n        if (!isValidResourceName(resourceName)) {\n            return "Error: Invalid resource name format";\n        }\n        \n        // Check if resource exists in whitelist\n        auto it = allowedResources.find(resourceName);\n        if (it == allowedResources.end()) {\n            return "Error: Resource not found or access denied";\n        }\n        \n        // Return resource content (in real system, would read from file safely)\n        return "Access granted to: " + resourceName + " - " + it->second;\n    }\n    \n    // List available resources\n    std::vector<std::string> listResources() const {\n        std::vector<std::string> resources;\n        for (const auto& pair : allowedResources) {\n            resources.push_back(pair.first);\n        }\n        return resources;\n    }\n};\n\n// Safe input reading with length validation\nstd::string safeReadLine(size_t maxLength = 256) {\n    std::string input;\n    if (!std::getline(std::cin, input)) {\n        return "";\n    }\n    \n    // Enforce maximum length\n    if (input.length() > maxLength) {\n        input = input.substr(0, maxLength);\n    }\n    \n    // Remove trailing whitespace\n    input.erase(std::find_if(input.rbegin(), input.rend(), \n        [](unsigned char ch) { return !std::isspace(ch); }).base(), input.end());\n    \n    return input;\n}\n\nint main() {\n    ResourceAccessControl rac;\n    \n    std::cout << "=== Resource Access Control System ===" << std::endl;\n    \n    // Test case 1: Valid resource access\n    std::cout << "\\n[Test 1] Accessing 'document1':" << std::endl;\n    std::cout << rac.accessResource("document1") << std::endl;\n    \n    // Test case 2: Another valid resource\n    std::cout << "\\n[Test 2] Accessing 'report_2024':" << std::endl;\n    std::cout << rac.accessResource("report_2024") << std::endl;\n    \n    // Test case 3: Path traversal attempt (blocked)\n    std::cout << "\\n[Test 3] Path traversal attempt '../etc/passwd':" << std::endl;\n    std::cout << rac.accessResource("../etc/passwd") << std::endl;\n    \n    // Test case 4: Non-existent resource\n    std::cout << "\\n[Test 4] Non-existent resource 'secret_data':" << std::endl;\n    std::cout << rac.accessResource("secret_data") << std::endl;\n    \n    // Test case 5: Invalid characters in resource name\n    std::cout << "\\n[Test 5] Invalid resource name 'doc/file':" << std::endl;\n    std::cout << rac.accessResource("doc/file") << std::endl;\n    \n    // List all available resources\n    std::cout << "\
[Available Resources]:" << std::endl;\n    std::vector<std::string> resources = rac.listResources();\n    for (const auto& res : resources) {\n        std::cout << "  - " << res << std::endl;
    }
    
    return 0;
}
