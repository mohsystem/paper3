
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.stream.Collectors;

class Task138 {
    static class User {
        private String username;
        private String passwordHash;
        private Set<String> roles;
        private Set<String> permissions;
        
        public User(String username, String password) {
            this.username = username;
            this.passwordHash = hashPassword(password);
            this.roles = new HashSet<>();
            this.permissions = new HashSet<>();
        }
        
        private String hashPassword(String password) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                byte[] hash = md.digest(password.getBytes());
                StringBuilder hexString = new StringBuilder();
                for (byte b : hash) {
                    String hex = Integer.toHexString(0xff & b);
                    if (hex.length() == 1) hexString.append('0');
                    hexString.append(hex);
                }
                return hexString.toString();
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException(e);
            }
        }
        
        public boolean verifyPassword(String password) {
            return this.passwordHash.equals(hashPassword(password));
        }
        
        public void addRole(String role) {
            this.roles.add(role);
        }
        
        public void removeRole(String role) {
            this.roles.remove(role);
        }
        
        public void addPermission(String permission) {
            this.permissions.add(permission);
        }
        
        public void removePermission(String permission) {
            this.permissions.remove(permission);
        }
        
        public boolean hasPermission(String permission) {
            return this.permissions.contains(permission);
        }
        
        public boolean hasRole(String role) {
            return this.roles.contains(role);
        }
        
        public String getUsername() {
            return username;
        }
        
        public Set<String> getRoles() {
            return new HashSet<>(roles);
        }
        
        public Set<String> getPermissions() {
            return new HashSet<>(permissions);
        }
    }
    
    static class PermissionManager {
        private Map<String, User> users;
        private Map<String, Set<String>> rolePermissions;
        
        public PermissionManager() {
            this.users = new HashMap<>();
            this.rolePermissions = new HashMap<>();
            initializeDefaultRoles();
        }
        
        private void initializeDefaultRoles() {
            rolePermissions.put("ADMIN", new HashSet<>(Arrays.asList(
                "CREATE_USER", "DELETE_USER", "MODIFY_USER", "READ_USER",
                "CREATE_ROLE", "DELETE_ROLE", "ASSIGN_ROLE"
            )));
            rolePermissions.put("MANAGER", new HashSet<>(Arrays.asList(
                "READ_USER", "MODIFY_USER", "ASSIGN_ROLE"
            )));
            rolePermissions.put("USER", new HashSet<>(Arrays.asList(
                "READ_USER"
            )));
        }
        
        public boolean createUser(String username, String password) {
            if (users.containsKey(username)) {
                return false;
            }
            users.put(username, new User(username, password));
            return true;
        }
        
        public boolean authenticateUser(String username, String password) {
            User user = users.get(username);
            if (user == null) {
                return false;
            }
            return user.verifyPassword(password);
        }
        
        public boolean assignRole(String username, String role) {
            User user = users.get(username);
            if (user == null || !rolePermissions.containsKey(role)) {
                return false;
            }
            user.addRole(role);
            Set<String> permissions = rolePermissions.get(role);
            for (String permission : permissions) {
                user.addPermission(permission);
            }
            return true;
        }
        
        public boolean revokeRole(String username, String role) {
            User user = users.get(username);
            if (user == null) {
                return false;
            }
            user.removeRole(role);
            recalculatePermissions(username);
            return true;
        }
        
        private void recalculatePermissions(String username) {
            User user = users.get(username);
            if (user == null) return;
            
            Set<String> allPermissions = new HashSet<>();
            for (String role : user.getRoles()) {
                Set<String> rolePerms = rolePermissions.get(role);
                if (rolePerms != null) {
                    allPermissions.addAll(rolePerms);
                }
            }
            
            user.permissions.clear();
            user.permissions.addAll(allPermissions);
        }
        
        public boolean grantPermission(String username, String permission) {
            User user = users.get(username);
            if (user == null) {
                return false;
            }
            user.addPermission(permission);
            return true;
        }
        
        public boolean revokePermission(String username, String permission) {
            User user = users.get(username);
            if (user == null) {
                return false;
            }
            user.removePermission(permission);
            return true;
        }
        
        public boolean checkPermission(String username, String permission) {
            User user = users.get(username);
            if (user == null) {
                return false;
            }
            return user.hasPermission(permission);
        }
        
        public Set<String> getUserPermissions(String username) {
            User user = users.get(username);
            if (user == null) {
                return new HashSet<>();
            }
            return user.getPermissions();
        }
        
        public Set<String> getUserRoles(String username) {
            User user = users.get(username);
            if (user == null) {
                return new HashSet<>();
            }
            return user.getRoles();
        }
    }
    
    public static void main(String[] args) {
        PermissionManager pm = new PermissionManager();
        
        // Test Case 1: Create users and authenticate
        System.out.println("Test Case 1: User Creation and Authentication");
        pm.createUser("alice", "password123");
        pm.createUser("bob", "securepass");
        System.out.println("Alice authentication: " + pm.authenticateUser("alice", "password123"));
        System.out.println("Bob wrong password: " + pm.authenticateUser("bob", "wrongpass"));
        System.out.println();
        
        // Test Case 2: Assign roles to users
        System.out.println("Test Case 2: Role Assignment");
        pm.assignRole("alice", "ADMIN");
        pm.assignRole("bob", "USER");
        System.out.println("Alice roles: " + pm.getUserRoles("alice"));
        System.out.println("Bob roles: " + pm.getUserRoles("bob"));
        System.out.println();
        
        // Test Case 3: Check permissions based on roles
        System.out.println("Test Case 3: Permission Checking");
        System.out.println("Alice has CREATE_USER: " + pm.checkPermission("alice", "CREATE_USER"));
        System.out.println("Bob has CREATE_USER: " + pm.checkPermission("bob", "CREATE_USER"));
        System.out.println("Bob has READ_USER: " + pm.checkPermission("bob", "READ_USER"));
        System.out.println();
        
        // Test Case 4: Grant and revoke specific permissions
        System.out.println("Test Case 4: Grant and Revoke Permissions");
        pm.grantPermission("bob", "MODIFY_USER");
        System.out.println("Bob permissions after grant: " + pm.getUserPermissions("bob"));
        pm.revokePermission("bob", "MODIFY_USER");
        System.out.println("Bob permissions after revoke: " + pm.getUserPermissions("bob"));
        System.out.println();
        
        // Test Case 5: Multiple roles and permission inheritance
        System.out.println("Test Case 5: Multiple Roles");
        pm.createUser("charlie", "charlie123");
        pm.assignRole("charlie", "USER");
        pm.assignRole("charlie", "MANAGER");
        System.out.println("Charlie roles: " + pm.getUserRoles("charlie"));
        System.out.println("Charlie permissions: " + pm.getUserPermissions("charlie"));
        pm.revokeRole("charlie", "MANAGER");
        System.out.println("Charlie permissions after revoking MANAGER: " + pm.getUserPermissions("charlie"));
    }
}
