
import java.util.*;
import java.util.regex.Pattern;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

class Task138 {
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,50}$");
    
    enum Permission {
        READ, WRITE, DELETE, ADMIN
    }
    
    static class User {
        private final String username;
        private final String passwordHash;
        private final String salt;
        private final Set<Permission> permissions;
        
        public User(String username, String password, Set<Permission> permissions) 
                throws IllegalArgumentException, NoSuchAlgorithmException {
            if (!isValidUsername(username)) {
                throw new IllegalArgumentException("Invalid username format");
            }
            if (!isValidPassword(password)) {
                throw new IllegalArgumentException("Password must be at least " + MIN_PASSWORD_LENGTH + " characters");
            }
            
            this.username = username;
            this.salt = generateSalt();
            this.passwordHash = hashPassword(password, this.salt);
            this.permissions = new HashSet<>(permissions);
        }
        
        public String getUsername() {
            return username;
        }
        
        public Set<Permission> getPermissions() {
            return new HashSet<>(permissions);
        }
        
        public boolean verifyPassword(String password) throws NoSuchAlgorithmException {
            if (password == null) return false;
            String hash = hashPassword(password, this.salt);
            return MessageDigest.isEqual(hash.getBytes(), this.passwordHash.getBytes());
        }
    }
    
    static class PermissionManager {
        private final Map<String, User> users;
        private final Map<String, Integer> loginAttempts;
        private static final int MAX_LOGIN_ATTEMPTS = 3;
        
        public PermissionManager() {
            this.users = new HashMap<>();
            this.loginAttempts = new HashMap<>();
        }
        
        public synchronized boolean addUser(String username, String password, Set<Permission> permissions) {
            try {
                if (username == null || password == null || permissions == null) {
                    return false;
                }
                
                if (users.containsKey(username)) {
                    return false;
                }
                
                User user = new User(username, password, permissions);
                users.put(username, user);
                return true;
            } catch (Exception e) {
                return false;
            }
        }
        
        public synchronized boolean authenticate(String username, String password) {
            try {
                if (username == null || password == null) {
                    return false;
                }
                
                if (loginAttempts.getOrDefault(username, 0) >= MAX_LOGIN_ATTEMPTS) {
                    return false;
                }
                
                User user = users.get(username);
                if (user == null) {
                    recordFailedAttempt(username);
                    return false;
                }
                
                if (user.verifyPassword(password)) {
                    loginAttempts.remove(username);
                    return true;
                } else {
                    recordFailedAttempt(username);
                    return false;
                }
            } catch (Exception e) {
                return false;
            }
        }
        
        public synchronized boolean hasPermission(String username, Permission permission) {
            if (username == null || permission == null) {
                return false;
            }
            
            User user = users.get(username);
            if (user == null) {
                return false;
            }
            
            return user.getPermissions().contains(permission) || 
                   user.getPermissions().contains(Permission.ADMIN);
        }
        
        public synchronized boolean grantPermission(String adminUsername, String targetUsername, Permission permission) {
            if (!hasPermission(adminUsername, Permission.ADMIN)) {
                return false;
            }
            
            User user = users.get(targetUsername);
            if (user == null || permission == null) {
                return false;
            }
            
            user.permissions.add(permission);
            return true;
        }
        
        public synchronized boolean revokePermission(String adminUsername, String targetUsername, Permission permission) {
            if (!hasPermission(adminUsername, Permission.ADMIN)) {
                return false;
            }
            
            User user = users.get(targetUsername);
            if (user == null || permission == null) {
                return false;
            }
            
            user.permissions.remove(permission);
            return true;
        }
        
        private void recordFailedAttempt(String username) {
            loginAttempts.put(username, loginAttempts.getOrDefault(username, 0) + 1);
        }
    }
    
    private static boolean isValidUsername(String username) {
        if (username == null || username.length() > MAX_USERNAME_LENGTH) {
            return false;
        }
        return USERNAME_PATTERN.matcher(username).matches();
    }
    
    private static boolean isValidPassword(String password) {
        return password != null && password.length() >= MIN_PASSWORD_LENGTH;
    }
    
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    private static String hashPassword(String password, String salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt.getBytes());
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    public static void main(String[] args) {
        PermissionManager manager = new PermissionManager();
        
        // Test Case 1: Add admin user
        Set<Permission> adminPerms = new HashSet<>(Arrays.asList(Permission.ADMIN));
        boolean result1 = manager.addUser("admin", "Admin@1234", adminPerms);
        System.out.println("Test 1 - Add admin user: " + (result1 ? "PASSED" : "FAILED"));
        
        // Test Case 2: Add regular user with READ permission
        Set<Permission> readPerms = new HashSet<>(Arrays.asList(Permission.READ));
        boolean result2 = manager.addUser("user1", "User@12345", readPerms);
        System.out.println("Test 2 - Add regular user: " + (result2 ? "PASSED" : "FAILED"));
        
        // Test Case 3: Authenticate user
        boolean result3 = manager.authenticate("admin", "Admin@1234");
        System.out.println("Test 3 - Authenticate admin: " + (result3 ? "PASSED" : "FAILED"));
        
        // Test Case 4: Check permission
        boolean result4 = manager.hasPermission("user1", Permission.READ);
        System.out.println("Test 4 - User1 has READ permission: " + (result4 ? "PASSED" : "FAILED"));
        
        // Test Case 5: Grant permission (admin grants WRITE to user1)
        boolean result5 = manager.grantPermission("admin", "user1", Permission.WRITE);
        boolean hasWrite = manager.hasPermission("user1", Permission.WRITE);
        System.out.println("Test 5 - Admin grants WRITE to user1: " + (result5 && hasWrite ? "PASSED" : "FAILED"));
    }
}
