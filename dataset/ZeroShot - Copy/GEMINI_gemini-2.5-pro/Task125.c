#include <stdio.h>
#include <string.h>

// For consistency with instructions, we can note the conceptual name.
// Main class name: Task125

// --- Data Structures ---

// Define a whitelist of available resources
typedef struct {
    const char* id;
    const char* name;
} Resource;

static const Resource resources[] = {
    {"1", "Public Document"},
    {"2", "User Profile"},
    {"3", "Admin Panel"},
    {"4", "System Logs"}
};
static const int NUM_RESOURCES = sizeof(resources) / sizeof(resources[0]);

// Define role-based access control permissions
typedef struct {
    const char* roleName;
    const char** allowedResourceIds;
    int numPermissions;
} RolePermission;

static const char* guestPermissions[] = {"1"};
static const char* userPermissions[] = {"1", "2"};
static const char* adminPermissions[] = {"1", "2", "3", "4"};

static const RolePermission rolePermissions[] = {
    {"GUEST", guestPermissions, 1},
    {"USER", userPermissions, 2},
    {"ADMIN", adminPermissions, 4}
};
static const int NUM_ROLES = sizeof(rolePermissions) / sizeof(rolePermissions[0]);


/**
 * Securely accesses a resource based on a user's role and requested resource ID.
 * The function returns a statically allocated string, so the caller should not free it.
 *
 * @param resourceId The identifier for the resource to be accessed.
 * @param userRole   The role of the user requesting access.
 * @return A constant string literal indicating the result of the access attempt.
 */
const char* accessResource(const char* resourceId, const char* userRole) {
    // 1. Validate user role
    const RolePermission* role = NULL;
    for (int i = 0; i < NUM_ROLES; ++i) {
        if (strcmp(rolePermissions[i].roleName, userRole) == 0) {
            role = &rolePermissions[i];
            break;
        }
    }
    if (role == NULL) {
        return "Error: Invalid user role provided.";
    }

    // 2. Validate resource ID against the whitelist
    const Resource* resource = NULL;
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        if (strcmp(resources[i].id, resourceId) == 0) {
            resource = &resources[i];
            break;
        }
    }
    if (resource == NULL) {
        return "Access Denied: Resource not found.";
    }

    // 3. Check if the user's role has permission for the resource
    int hasPermission = 0;
    for (int i = 0; i < role->numPermissions; ++i) {
        if (strcmp(role->allowedResourceIds[i], resourceId) == 0) {
            hasPermission = 1;
            break;
        }
    }

    if (hasPermission) {
        // This approach is not ideal for complex strings, but safe for this example.
        // It avoids dynamic allocation issues by using a static buffer.
        static char resultBuffer[256];
        snprintf(resultBuffer, sizeof(resultBuffer), "Access Granted. Now accessing: %s", resource->name);
        return resultBuffer;
    } else {
        return "Access Denied: You do not have permission to access this resource.";
    }
}

int main() {
    printf("Running test cases...\n");

    // Test Case 1: Guest accessing a public resource (should be granted)
    printf("Test 1 (GUEST, Resource '1'): %s\n", accessResource("1", "GUEST"));

    // Test Case 2: Guest trying to access an admin resource (should be denied)
    printf("Test 2 (GUEST, Resource '3'): %s\n", accessResource("3", "GUEST"));

    // Test Case 3: Admin accessing an admin resource (should be granted)
    printf("Test 3 (ADMIN, Resource '3'): %s\n", accessResource("3", "ADMIN"));

    // Test Case 4: User trying to access a non-existent resource (should be denied)
    printf("Test 4 (USER, Resource '99'): %s\n", accessResource("99", "USER"));

    // Test Case 5: User accessing their profile (should be granted)
    printf("Test 5 (USER, Resource '2'): %s\n", accessResource("2", "USER"));

    return 0;
}