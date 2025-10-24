
import java.util.*;

public class Task138 {
    private Map<String, User> users;
    private Map<String, Role> roles;
    
    public Task138() {
        users = new HashMap<>();
        roles = new HashMap<>();
        initializeDefaultRoles();
    }
    
    private void initializeDefaultRoles() {
        Role admin = new Role("admin");
        admin.addPermission("read");
        admin.addPermission("write");
        admin.addPermission("delete");
        admin.addPermission("execute");
        roles.put("admin", admin);
        
        Role editor = new Role("editor");
        editor.addPermission("read");
        editor.addPermission("write");
        roles.put("editor", editor);
        
        Role viewer = new Role("viewer");
        viewer.addPermission("read");
        roles.put("viewer", viewer);
    }
    
    public boolean addUser(String username, String roleName) {
        if (users.containsKey(username)) {
            return false;
        }
        Role role = roles.get(roleName);
        if (role == null) {
            return false;
        }
        users.put(username, new User(username, role));
        return true;
    }
    
    public boolean removeUser(String username) {
        return users.remove(username) != null;
    }
    
    public boolean hasPermission(String username, String permission) {
        User user = users.get(username);
        if (user == null) {
            return false;
        }
        return user.hasPermission(permission);
    }
    
    public boolean assignRole(String username, String roleName) {
        User user = users.get(username);
        Role role = roles.get(roleName);
        if (user == null || role == null) {
            return false;
        }
        user.setRole(role);
        return true;
    }
    
    public Set<String> getUserPermissions(String username) {
        User user = users.get(username);
        if (user == null) {
            return new HashSet<>();
        }
        return user.getPermissions();
    }
    
    public boolean addPermissionToRole(String roleName, String permission) {
        Role role = roles.get(roleName);
        if (role == null) {
            return false;
        }
        role.addPermission(permission);
        return true;
    }
    
    public boolean removePermissionFromRole(String roleName, String permission) {
        Role role = roles.get(roleName);
        if (role == null) {
            return false;
        }
        return role.removePermission(permission);
    }
    
    public String getUserRole(String username) {
        User user = users.get(username);
        if (user == null) {
            return null;
        }
        return user.getRole().getName();
    }
    
    static class User {
        private String username;
        private Role role;
        
        public User(String username, Role role) {
            this.username = username;
            this.role = role;
        }
        
        public boolean hasPermission(String permission) {
            return role.hasPermission(permission);
        }
        
        public Set<String> getPermissions() {
            return role.getPermissions();
        }
        
        public Role getRole() {
            return role;
        }
        
        public void setRole(Role role) {
            this.role = role;
        }
    }
    
    static class Role {
        private String name;
        private Set<String> permissions;
        
        public Role(String name) {
            this.name = name;
            this.permissions = new HashSet<>();
        }
        
        public void addPermission(String permission) {
            permissions.add(permission);
        }
        
        public boolean removePermission(String permission) {
            return permissions.remove(permission);
        }
        
        public boolean hasPermission(String permission) {
            return permissions.contains(permission);
        }
        
        public Set<String> getPermissions() {
            return new HashSet<>(permissions);
        }
        
        public String getName() {
            return name;
        }
    }
    
    public static void main(String[] args) {
        Task138 system = new Task138();
        
        // Test Case 1: Add users with different roles
        System.out.println("Test Case 1: Add users");
        System.out.println("Add admin user: " + system.addUser("john", "admin"));
        System.out.println("Add editor user: " + system.addUser("jane", "editor"));
        System.out.println("Add viewer user: " + system.addUser("bob", "viewer"));
        System.out.println();
        
        // Test Case 2: Check permissions
        System.out.println("Test Case 2: Check permissions");
        System.out.println("john has delete permission: " + system.hasPermission("john", "delete"));
        System.out.println("jane has delete permission: " + system.hasPermission("jane", "delete"));
        System.out.println("bob has read permission: " + system.hasPermission("bob", "read"));
        System.out.println("bob has write permission: " + system.hasPermission("bob", "write"));
        System.out.println();
        
        // Test Case 3: Get user permissions
        System.out.println("Test Case 3: Get user permissions");
        System.out.println("john's permissions: " + system.getUserPermissions("john"));
        System.out.println("jane's permissions: " + system.getUserPermissions("jane"));
        System.out.println("bob's permissions: " + system.getUserPermissions("bob"));
        System.out.println();
        
        // Test Case 4: Change user role
        System.out.println("Test Case 4: Change user role");
        System.out.println("bob's current role: " + system.getUserRole("bob"));
        System.out.println("Assign editor role to bob: " + system.assignRole("bob", "editor"));
        System.out.println("bob's new role: " + system.getUserRole("bob"));
        System.out.println("bob now has write permission: " + system.hasPermission("bob", "write"));
        System.out.println();
        
        // Test Case 5: Add and remove permissions from role
        System.out.println("Test Case 5: Modify role permissions");
        System.out.println("Add 'backup' permission to viewer: " + system.addPermissionToRole("viewer", "backup"));
        system.addUser("alice", "viewer");
        System.out.println("alice has backup permission: " + system.hasPermission("alice", "backup"));
        System.out.println("Remove 'backup' permission from viewer: " + system.removePermissionFromRole("viewer", "backup"));
        System.out.println("alice still has backup permission: " + system.hasPermission("alice", "backup"));
    }
}
