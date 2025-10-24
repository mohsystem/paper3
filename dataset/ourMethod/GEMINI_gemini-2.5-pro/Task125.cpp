#include <iostream>
#include <string>
#include <vector>
#include <set>

// Use a std::set for efficient, case-sensitive lookups of allowed resources.
const std::set<std::string> ALLOWED_RESOURCES = {
    "PROFILE", "DASHBOARD", "SETTINGS", "REPORTS"
};

/**
 * @brief Checks if a user has access to a specific resource.
 * This function validates the input and compares it against a safe list of
 * allowed resources to prevent unauthorized access.
 *
 * @param resourceId The ID of the resource being requested.
 * @return A string indicating whether access is granted or denied.
 */
std::string getResourceAccess(const std::string& resourceId) {
    // Rule#6: Ensure all input is validated.
    // std::string::empty() is a safe way to check.
    if (resourceId.empty()) {
        return "Access Denied: Invalid resource ID.";
    }
    
    // Check against the predefined set. This acts as a whitelist.
    if (ALLOWED_RESOURCES.count(resourceId)) {
        // Safe string concatenation using std::string, which handles memory.
        return "Access Granted to resource: " + resourceId;
    } else {
        // Return a generic error to avoid information disclosure.
        return "Access Denied: Resource not found or permission denied.";
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: A valid, allowed resource.
    std::string resource1 = "DASHBOARD";
    std::cout << "Requesting '" << resource1 << "': " << getResourceAccess(resource1) << std::endl;

    // Test Case 2: Another valid, allowed resource.
    std::string resource2 = "SETTINGS";
    std::cout << "Requesting '" << resource2 << "': " << getResourceAccess(resource2) << std::endl;

    // Test Case 3: An invalid/unauthorized resource.
    std::string resource3 = "ADMIN_PANEL";
    std::cout << "Requesting '" << resource3 << "': " << getResourceAccess(resource3) << std::endl;

    // Test Case 4: An empty string input.
    std::string resource4 = "";
    std::cout << "Requesting 'empty string': " << getResourceAccess(resource4) << std::endl;
    
    // Test Case 5: A different invalid resource (demonstrates case-sensitivity).
    std::string resource5 = "profile"; 
    std::cout << "Requesting '" << resource5 << "': " << getResourceAccess(resource5) << std::endl;

    std::cout << "--- Test Cases Finished ---" << std::endl;
    
    return 0;
}