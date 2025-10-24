import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class Task138 {

    /**
     * Manages user roles and permissions.
     * This class is designed to be secure by:
     * - Encapsulating data: all internal maps are private.
     * - Validating input: methods check for null or empty strings.
     * - Using appropriate data structures (Set) to prevent duplicates.
     */
    static class PermissionManager {
        // Maps role names to a set of permission strings
        private final Map<String, Set<String>> rolesToPermissions;
        // Maps user names to a set of role strings
        private final Map<String, Set<String>> usersToRoles;

        public PermissionManager() {
            this.rolesToPermissions = new HashMap<>();
            this.usersToRoles = new HashMap<>();
        }

        /**
         * Adds a new role to the system.
         * @param roleName The name of the role to add.
         * @return true if the role was added successfully, false if the input is invalid or the role already exists.
         */
        public boolean addRole(String roleName) {
            if (roleName == null || roleName.trim().isEmpty()) {
                return false; // Invalid input
            }
            if (rolesToPermissions.containsKey(roleName)) {
                return false; // Role already exists
            }
            rolesToPermissions.put(roleName, new HashSet<>());
            return true;
        }

        /**
         * Adds a new user to the system.
         * @param userName The name of the user to add.
         * @return true if the user was added successfully, false if the input is invalid or the user already exists.
         */
        public boolean addUser(String userName) {
            if (userName == null || userName.trim().isEmpty()) {
                return false; // Invalid input
            }
            if (usersToRoles.containsKey(userName)) {
                return false; // User already exists
            }
            usersToRoles.put(userName, new HashSet<>());
            return true;
        }

        /**
         * Adds a permission to an existing role.
         * @param roleName The role to modify.
         * @param permission The permission to add.
         * @return true if successful, false if role doesn't exist or input is invalid.
         */
        public boolean addPermissionToRole(String roleName, String permission) {
            if (roleName == null || roleName.trim().isEmpty() || permission == null || permission.trim().isEmpty()) {
                return false; // Invalid input
            }
            if (!rolesToPermissions.containsKey(roleName)) {
                return false; // Role does not exist
            }
            rolesToPermissions.get(roleName).add(permission);
            return true;
        }

        /**
         * Assigns an existing role to an existing user.
         * @param userName The user to whom the role will be assigned.
         * @param roleName The role to assign.
         * @return true if successful, false if user or role doesn't exist or input is invalid.
         */
        public boolean assignRoleToUser(String userName, String roleName) {
            if (userName == null || userName.trim().isEmpty() || roleName == null || roleName.trim().isEmpty()) {
                return false; // Invalid input
            }
            if (!usersToRoles.containsKey(userName) || !rolesToPermissions.containsKey(roleName)) {
                return false; // User or Role does not exist
            }
            usersToRoles.get(userName).add(roleName);
            return true;
        }

        /**
         * Checks if a user has a specific permission.
         * @param userName The user to check.
         * @param permission The permission to check for.
         * @return true if the user has the permission through any of their roles, false otherwise.
         */
        public boolean checkPermission(String userName, String permission) {
            if (userName == null || permission == null || !usersToRoles.containsKey(userName)) {
                return false;
            }

            Set<String> userRoles = usersToRoles.get(userName);
            if (userRoles == null) {
                return false;
            }

            for (String role : userRoles) {
                Set<String> permissions = rolesToPermissions.get(role);
                if (permissions != null && permissions.contains(permission)) {
                    return true;
                }
            }
            return false;
        }
    }

    public static void main(String[] args) {
        PermissionManager manager = new PermissionManager();

        // Setup
        manager.addRole("admin");
        manager.addRole("editor");
        manager.addRole("viewer");

        manager.addPermissionToRole("admin", "read");
        manager.addPermissionToRole("admin", "write");
        manager.addPermissionToRole("admin", "delete");
        manager.addPermissionToRole("admin", "manage_users");

        manager.addPermissionToRole("editor", "read");
        manager.addPermissionToRole("editor", "write");

        manager.addPermissionToRole("viewer", "read");

        manager.addUser("alice");
        manager.addUser("bob");
        manager.addUser("charlie");

        manager.assignRoleToUser("alice", "admin");
        manager.assignRoleToUser("bob", "editor");
        manager.assignRoleToUser("charlie", "viewer");

        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Admin has delete permission
        System.out.println("1. Alice has 'delete' permission: " + manager.checkPermission("alice", "delete"));

        // Test Case 2: Editor has write permission
        System.out.println("2. Bob has 'write' permission: " + manager.checkPermission("bob", "write"));

        // Test Case 3: Editor does NOT have delete permission
        System.out.println("3. Bob has 'delete' permission: " + manager.checkPermission("bob", "delete"));

        // Test Case 4: Non-existent user
        System.out.println("4. Dave has 'read' permission: " + manager.checkPermission("dave", "read"));
        
        // Test Case 5: User with multiple roles
        manager.assignRoleToUser("bob", "viewer"); // bob is now editor and viewer
        System.out.println("5. Bob (now editor & viewer) has 'read' permission: " + manager.checkPermission("bob", "read"));

        System.out.println("--- Test Cases Finished ---");
    }
}