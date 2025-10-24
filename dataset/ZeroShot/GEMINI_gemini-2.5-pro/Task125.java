import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.Arrays;

public class Task125 {

    // Define a whitelist of available resources
    private static final Map<String, String> resources = new HashMap<>();
    // Define role-based access control permissions
    private static final Map<String, Set<String>> rolePermissions = new HashMap<>();

    static {
        // Initialize resources
        resources.put("1", "Public Document");
        resources.put("2", "User Profile");
        resources.put("3", "Admin Panel");
        resources.put("4", "System Logs");

        // Initialize permissions for roles
        // GUEST can only access resource "1"
        rolePermissions.put("GUEST", new HashSet<>(Arrays.asList("1")));
        // USER can access resources "1" and "2"
        rolePermissions.put("USER", new HashSet<>(Arrays.asList("1", "2")));
        // ADMIN can access all resources
        rolePermissions.put("ADMIN", new HashSet<>(Arrays.asList("1", "2", "3", "4")));
    }

    /**
     * Securely accesses a resource based on a user's role and requested resource ID.
     *
     * @param resourceId The identifier for the resource to be accessed.
     * @param userRole   The role of the user requesting access.
     * @return A string indicating the result of the access attempt.
     */
    public static String accessResource(String resourceId, String userRole) {
        // 1. Validate user role
        if (!rolePermissions.containsKey(userRole)) {
            return "Error: Invalid user role provided.";
        }

        // 2. Validate resource ID against the whitelist
        if (!resources.containsKey(resourceId)) {
            return "Access Denied: Resource not found.";
        }

        // 3. Check if the user's role has permission for the resource
        Set<String> allowedResources = rolePermissions.get(userRole);
        if (allowedResources != null && allowedResources.contains(resourceId)) {
            String resourceName = resources.get(resourceId);
            return "Access Granted. Now accessing: " + resourceName;
        } else {
            return "Access Denied: You do not have permission to access this resource.";
        }
    }

    public static void main(String[] args) {
        System.out.println("Running test cases...");

        // Test Case 1: Guest accessing a public resource (should be granted)
        System.out.println("Test 1 (GUEST, Resource '1'): " + accessResource("1", "GUEST"));

        // Test Case 2: Guest trying to access an admin resource (should be denied)
        System.out.println("Test 2 (GUEST, Resource '3'): " + accessResource("3", "GUEST"));

        // Test Case 3: Admin accessing an admin resource (should be granted)
        System.out.println("Test 3 (ADMIN, Resource '3'): " + accessResource("3", "ADMIN"));

        // Test Case 4: User trying to access a non-existent resource (should be denied)
        System.out.println("Test 4 (USER, Resource '99'): " + accessResource("99", "USER"));

        // Test Case 5: User accessing their profile (should be granted)
        System.out.println("Test 5 (USER, Resource '2'): " + accessResource("2", "USER"));
    }
}