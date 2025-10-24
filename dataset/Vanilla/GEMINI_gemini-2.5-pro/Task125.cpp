#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <algorithm>

// Function to convert a string to lowercase
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

/**
 * Accesses a resource based on the provided resource name.
 * @param resourceName The name of the resource to access.
 * @return The resource content if found, otherwise an error message.
 */
std::string getResource(const std::string& resourceName) {
    std::map<std::string, std::string> resources;
    resources["profile"] = "User Profile Data: { 'user': 'John Doe', 'email': 'john.doe@example.com' }";
    resources["settings"] = "Account Settings Page: [ 'Theme: Dark', 'Language: English' ]";
    resources["dashboard"] = "Main Dashboard View: Widgets and Stats";
    resources["logs"] = "System Activity Logs: [ 'INFO: User login', 'WARN: Low disk space' ]";

    std::string lowerResourceName = toLower(resourceName);
    auto it = resources.find(lowerResourceName);

    if (it != resources.end()) {
        return "Accessing " + resourceName + ": " + it->second;
    } else {
        return "Access Denied: Resource '" + resourceName + "' not found.";
    }
}

int main() {
    // Test Cases
    std::string testCases[] = {"profile", "dashboard", "admin", "settings", "data"};
    
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Input: " << testCase << std::endl;
        std::string result = getResource(testCase);
        std::cout << "Output: " << result << std::endl << std::endl;
    }

    return 0;
}