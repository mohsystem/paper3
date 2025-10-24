import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class Task138 {

    // Enum for type-safe permissions
    enum Permission {
        READ,
        WRITE,
        DELETE,
        EXECUTE
    }

    // Manages users, roles, and permissions
    static class PermissionManager {
        private final Map<String, Set<Permission>> roles; // Role name -> Set of permissions
        private final Map<String, String> users;          // User name -> Role name

        public PermissionManager() {
            this.roles = new HashMap<>();
            this.users = new HashMap<>();
        }

        // Add a new role with a set of permissions
        public void addRole(String roleName, Set<Permission> permissions) {
            if (roleName != null && !roleName.trim().isEmpty() && permissions != null) {
                roles.put(roleName, new HashSet<>(permissions));
            }
        }

        // Assign a role to a user
        public void assignRoleToUser(String userName, String roleName) {
            // Ensure the role exists before assigning
            if (userName != null && !userName.trim().isEmpty() && roles.containsKey(roleName)) {
                users.put(userName, roleName);
            }
        }

        /**
         * Checks if a user has a specific permission.
         *
         * @param userName   The name of the user.
         * @param permission The permission to check.
         * @return true if the user has the permission, false otherwise.
         */
        public boolean checkPermission(String userName, Permission permission) {
            if (userName == null || permission == null) {
                return false;
            }

            // 1. Find the user's role
            String roleName = users.get(userName);
            if (roleName == null) {
                // User does not exist or has no role
                return false;
            }

            // 2. Get the permissions for that role
            Set<Permission> permissions = roles.get(roleName);
            if (permissions == null) {
                // Role exists for user, but role definition is missing (should not happen with proper setup)
                return false;
            }

            // 3. Check if the permission is in the set
            return permissions.contains(permission);
        }
    }

    public static void main(String[] args) {
        PermissionManager manager = new PermissionManager();

        // 1. Define roles and their permissions
        manager.addRole("ADMIN", Set.of(Permission.READ, Permission.WRITE, Permission.DELETE, Permission.EXECUTE));
        manager.addRole("EDITOR", Set.of(Permission.READ, Permission.WRITE));
        manager.addRole("VIEWER", Set.of(Permission.READ));

        // 2. Create users and assign roles
        manager.assignRoleToUser("alice", "ADMIN");
        manager.assignRoleToUser("bob", "EDITOR");
        manager.assignRoleToUser("charlie", "VIEWER");

        System.out.println("Running test cases...\n");

        // Test Case 1: Admin user checking an admin-only permission
        System.out.println("Test 1: Does alice (ADMIN) have DELETE permission?");
        System.out.println("Result: " + manager.checkPermission("alice", Permission.DELETE)); // Expected: true

        // Test Case 2: Editor user checking an editor permission
        System.out.println("\nTest 2: Does bob (EDITOR) have WRITE permission?");
        System.out.println("Result: " + manager.checkPermission("bob", Permission.WRITE)); // Expected: true

        // Test Case 3: Editor user checking an admin-only permission
        System.out.println("\nTest 3: Does bob (EDITOR) have DELETE permission?");
        System.out.println("Result: " + manager.checkPermission("bob", Permission.DELETE)); // Expected: false

        // Test Case 4: Viewer user checking a read-only permission
        System.out.println("\nTest 4: Does charlie (VIEWER) have READ permission?");
        System.out.println("Result: " + manager.checkPermission("charlie", Permission.READ)); // Expected: true

        // Test Case 5: Non-existent user checking any permission
        System.out.println("\nTest 5: Does david (non-existent) have READ permission?");
        System.out.println("Result: " + manager.checkPermission("david", Permission.READ)); // Expected: false
    }
}