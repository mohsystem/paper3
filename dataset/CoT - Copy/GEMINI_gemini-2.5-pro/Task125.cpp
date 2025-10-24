#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// A whitelist of allowed resources and their internal paths.
// This prevents path traversal as user input is only used as a key.
const std::unordered_map<std::string, std::string> allowedResources = {
    {"profile", "data/user_profile.dat"},
    {"settings", "config/app_settings.json"},
    {"dashboard", "pages/dashboard.html"}
};

/**
 * Securely accesses a resource based on a predefined whitelist.
 *
 * @param resourceKey The user-provided key for the resource.
 * @return A string indicating the result of the access attempt.
 */
std::string accessResource(const std::string& resourceKey) {
    // Basic input validation
    if (resourceKey.empty()) {
        return "Error: Resource key cannot be empty.";
    }

    // Securely check if the key exists in our whitelist map.
    auto it = allowedResources.find(resourceKey);

    if (it != allowedResources.end()) {
        // 'it->second' is the safe, internal path.
        // In a real application, you would use this path to read the file.
        // For this example, we just simulate the access.
        return "Success: Accessing resource at " + it->second;
    } else {
        // Generic error message to avoid leaking information.
        return "Error: Resource not found or access denied.";
    }
}

int main() {
    std::cout << "CPP Test Cases:" << std::endl;
    
    std::vector<std::string> testCases = {
        "profile",          // Valid case 1
        "settings",         // Valid case 2
        "logs",             // Invalid case
        "../etc/passwd",    // Path traversal attempt
        ""                  // Empty input
    };

    for (const auto& test : testCases) {
        std::cout << "Input: \"" << test << "\" -> Output: " << accessResource(test) << std::endl;
    }

    return 0;
}