import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

class PermissionManager {
    private final Map<String, Set<String>> rolesToPermissions;
    private final Map<String, Set<String>> usersToRoles;

    public PermissionManager() {
        this.rolesToPermissions = new HashMap<>();
        this.usersToRoles = new HashMap<>();
    }

    public boolean addUser(String username) {
        if (username == null || username.trim().isEmpty()) {
            System.err.println("Error: Username cannot be null or empty.");
            return false;
        }
        if (usersToRoles.containsKey(username)) {
            System.err.println("Error: User '" + username + "' already exists.");
            return false;
        }
        usersToRoles.put(username, new HashSet<>());
        return true;
    }

    public boolean addRole(String roleName, Set<String> permissions) {
        if (roleName == null || roleName.trim().isEmpty()) {
            System.err.println("Error: Role name cannot be null or empty.");
            return false;
        }
        if (permissions == null || permissions.isEmpty()) {
            System.err.println("Error: Permissions set cannot be null or empty.");
            return false;
        }
        // Defensive copy of the permissions set
        rolesToPermissions.put(roleName, new HashSet<>(permissions));
        return true;
    }

    public boolean assignRoleToUser(String username, String roleName) {
        if (username == null || !usersToRoles.containsKey(username)) {
            System.err.println("Error: User '" + (username == null ? "NULL" : username) + "' not found.");
            return false;
        }
        if (roleName == null || !rolesToPermissions.containsKey(roleName)) {
            System.err.println("Error: Role '" + (roleName == null ? "NULL" : roleName) + "' not found.");
            return false;
        }
        usersToRoles.get(username).add(roleName);
        return true;
    }

    public boolean checkPermission(String username, String permission) {
        if (username == null || !usersToRoles.containsKey(username)) {
            // Fail closed: if user doesn't exist, they have no permissions.
            return false;
        }
        if (permission == null || permission.trim().isEmpty()) {
            return false;
        }

        Set<String> userRoles = usersToRoles.get(username);
        if (userRoles == null) {
            return false;
        }

        for (String roleName : userRoles) {
            Set<String> permissions = rolesToPermissions.get(roleName);
            if (permissions != null && permissions.contains(permission)) {
                return true;
            }
        }

        return false;
    }
}

public class Task138 {
    public static void main(String[] args) {
        PermissionManager pm = new PermissionManager();

        System.out.println("--- Test Case 1: Basic Setup and Permission Check ---");
        pm.addRole("admin", new HashSet<>(Set.of("read", "write", "delete")));
        pm.addRole("editor", new HashSet<>(Set.of("read", "write")));
        pm.addRole("viewer", new HashSet<>(Set.of("read")));
        pm.addUser("alice");
        pm.addUser("bob");
        pm.assignRoleToUser("alice", "admin");
        pm.assignRoleToUser("bob", "editor");

        System.out.println("Does alice have 'delete' permission? " + pm.checkPermission("alice", "delete")); // Expected: true
        System.out.println("Does bob have 'delete' permission? " + pm.checkPermission("bob", "delete"));   // Expected: false
        System.out.println("Does bob have 'write' permission? " + pm.checkPermission("bob", "write"));   // Expected: true
        System.out.println();

        System.out.println("--- Test Case 2: User with multiple roles ---");
        pm.addUser("charlie");
        pm.assignRoleToUser("charlie", "viewer");
        pm.assignRoleToUser("charlie", "editor"); // Promote charlie
        System.out.println("Does charlie have 'write' permission? " + pm.checkPermission("charlie", "write")); // Expected: true
        System.out.println("Does charlie have 'read' permission? " + pm.checkPermission("charlie", "read")); // Expected: true
        System.out.println();

        System.out.println("--- Test Case 3: Non-existent user, role, or permission ---");
        System.out.println("Does non_existent_user have 'read' permission? " + pm.checkPermission("non_existent_user", "read")); // Expected: false
        System.out.println("Does alice have 'execute' permission? " + pm.checkPermission("alice", "execute")); // Expected: false
        System.out.println("Assigning non-existent role: " + pm.assignRoleToUser("alice", "super_admin")); // Expected: false (with error)
        System.out.println();

        System.out.println("--- Test Case 4: User with no roles ---");
        pm.addUser("dave");
        System.out.println("Does dave have 'read' permission? " + pm.checkPermission("dave", "read")); // Expected: false
        System.out.println();
        
        System.out.println("--- Test Case 5: Invalid inputs ---");
        System.out.println("Adding user with null name: " + pm.addUser(null)); // Expected: false (with error)
        System.out.println("Adding user with empty name: " + pm.addUser("  ")); // Expected: false (with error)
        System.out.println("Checking null permission for alice: " + pm.checkPermission("alice", null)); // Expected: false
        System.out.println();
    }
}