#include <iostream>
#include <string>
#include <map>
#include <set>

// In C++, the concept of a main class like in Java isn't standard.
// We use a namespace or simply define functions and use a main() function.
// For consistency with the instructions, we can simulate it with a struct/class or namespace.
// We'll call it Task125.

class Task125 {
private:
    // Define a whitelist of available resources
    static const std::map<std::string, std::string> resources;
    // Define role-based access control permissions
    static const std::map<std::string, std::set<std::string>> rolePermissions;

public:
    /**
     * Securely accesses a resource based on a user's role and requested resource ID.
     *
     * @param resourceId The identifier for the resource to be accessed.
     * @param userRole   The role of the user requesting access.
     * @return A string indicating the result of the access attempt.
     */
    static std::string accessResource(const std::string& resourceId, const std::string& userRole) {
        // 1. Validate user role
        auto roleIt = rolePermissions.find(userRole);
        if (roleIt == rolePermissions.end()) {
            return "Error: Invalid user role provided.";
        }

        // 2. Validate resource ID against the whitelist
        auto resourceIt = resources.find(resourceId);
        if (resourceIt == resources.end()) {
            return "Access Denied: Resource not found.";
        }

        // 3. Check if the user's role has permission for the resource
        const std::set<std::string>& allowedResources = roleIt->second;
        if (allowedResources.count(resourceId)) {
            return "Access Granted. Now accessing: " + resourceIt->second;
        } else {
            return "Access Denied: You do not have permission to access this resource.";
        }
    }
};

// Initialize static members
const std::map<std::string, std::string> Task125::resources = {
    {"1", "Public Document"},
    {"2", "User Profile"},
    {"3", "Admin Panel"},
    {"4", "System Logs"}
};

const std::map<std::string, std::set<std::string>> Task125::rolePermissions = {
    {"GUEST", {"1"}},
    {"USER", {"1", "2"}},
    {"ADMIN", {"1", "2", "3", "4"}}
};


int main() {
    std::cout << "Running test cases..." << std::endl;

    // Test Case 1: Guest accessing a public resource (should be granted)
    std::cout << "Test 1 (GUEST, Resource '1'): " << Task125::accessResource("1", "GUEST") << std::endl;

    // Test Case 2: Guest trying to access an admin resource (should be denied)
    std::cout << "Test 2 (GUEST, Resource '3'): " << Task125::accessResource("3", "GUEST") << std::endl;

    // Test Case 3: Admin accessing an admin resource (should be granted)
    std::cout << "Test 3 (ADMIN, Resource '3'): " << Task125::accessResource("3", "ADMIN") << std::endl;

    // Test Case 4: User trying to access a non-existent resource (should be denied)
    std::cout << "Test 4 (USER, Resource '99'): " << Task125::accessResource("99", "USER") << std::endl;

    // Test Case 5: User accessing their profile (should be granted)
    std::cout << "Test 5 (USER, Resource '2'): " << Task125::accessResource("2", "USER") << std::endl;

    return 0;
}