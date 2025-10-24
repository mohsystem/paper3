import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

// Enum for different types of permissions
enum Permission {
    READ, WRITE, EXECUTE, DELETE
}

// Represents a user with a username and a set of permissions
class User {
    private String username;
    private Set<Permission> permissions;

    public User(String username) {
        this.username = username;
        this.permissions = new HashSet<>();
    }

    public String getUsername() {
        return username;
    }

    public Set<Permission> getPermissions() {
        return permissions;
    }

    public void grantPermission(Permission permission) {
        permissions.add(permission);
    }

    public void revokePermission(Permission permission) {
        permissions.remove(permission);
    }

    public boolean hasPermission(Permission permission) {
        return permissions.contains(permission);
    }
}

// Manages all users and their permissions
class PermissionManager {
    private Map<String, User> users;

    public PermissionManager() {
        this.users = new HashMap<>();
    }

    public void addUser(String username) {
        if (!users.containsKey(username)) {
            users.put(username, new User(username));
            System.out.println("User '" + username + "' added.");
        } else {
            System.out.println("User '" + username + "' already exists.");
        }
    }

    public void grantPermission(String username, Permission permission) {
        User user = users.get(username);
        if (user != null) {
            user.grantPermission(permission);
            System.out.println("Permission " + permission + " granted to '" + username + "'.");
        } else {
            System.out.println("User '" + username + "' not found.");
        }
    }

    public void revokePermission(String username, Permission permission) {
        User user = users.get(username);
        if (user != null) {
            user.revokePermission(permission);
            System.out.println("Permission " + permission + " revoked from '" + username + "'.");
        } else {
            System.out.println("User '" + username + "' not found.");
        }
    }

    public boolean hasPermission(String username, Permission permission) {
        User user = users.get(username);
        return user != null && user.hasPermission(permission);
    }

    public void printUserPermissions(String username) {
        User user = users.get(username);
        if (user != null) {
            if (user.getPermissions().isEmpty()) {
                System.out.println("User '" + username + "' has no permissions.");
            } else {
                String perms = user.getPermissions().stream()
                                  .map(Enum::name)
                                  .sorted()
                                  .collect(Collectors.joining(", "));
                System.out.println("User '" + username + "' permissions: " + perms);
            }
        } else {
            System.out.println("User '" + username + "' not found.");
        }
    }
}

// Main class with test cases
public class Task138 {
    public static void main(String[] args) {
        PermissionManager manager = new PermissionManager();

        System.out.println("--- Test Case 1: Add users ---");
        manager.addUser("alice");
        manager.addUser("bob");
        System.out.println();

        System.out.println("--- Test Case 2: Grant permissions to alice ---");
        manager.grantPermission("alice", Permission.READ);
        manager.grantPermission("alice", Permission.WRITE);
        manager.printUserPermissions("alice");
        manager.printUserPermissions("bob");
        System.out.println();

        System.out.println("--- Test Case 3: Check alice's permissions ---");
        System.out.println("Does alice have WRITE permission? " + manager.hasPermission("alice", Permission.WRITE));
        System.out.println("Does alice have EXECUTE permission? " + manager.hasPermission("alice", Permission.EXECUTE));
        System.out.println();

        System.out.println("--- Test Case 4: Revoke a permission from alice ---");
        manager.revokePermission("alice", Permission.WRITE);
        manager.printUserPermissions("alice");
        System.out.println();
        
        System.out.println("--- Test Case 5: Final permission checks ---");
        System.out.println("Does alice have WRITE permission now? " + manager.hasPermission("alice", Permission.WRITE));
        System.out.println("Does alice still have READ permission? " + manager.hasPermission("alice", Permission.READ));
        System.out.println("Does bob have READ permission? " + manager.hasPermission("bob", Permission.READ));
        manager.grantPermission("bob", Permission.EXECUTE);
        manager.printUserPermissions("bob");
    }
}