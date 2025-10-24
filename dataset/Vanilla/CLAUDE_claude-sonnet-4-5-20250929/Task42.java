
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task42 {
    private Map<String, UserCredential> users;
    
    static class UserCredential {
        String salt;
        String hashedPassword;
        
        UserCredential(String salt, String hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }
    
    public Task42() {
        this.users = new HashMap<>();
    }
    
    public String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public boolean registerUser(String username, String password) {
        if (users.containsKey(username)) {
            return false;
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        users.put(username, new UserCredential(salt, hashedPassword));
        return true;
    }
    
    public boolean authenticateUser(String username, String password) {
        if (!users.containsKey(username)) {
            return false;
        }
        
        UserCredential credential = users.get(username);
        String hashedPassword = hashPassword(password, credential.salt);
        return hashedPassword.equals(credential.hashedPassword);
    }
    
    public boolean deleteUser(String username) {
        if (!users.containsKey(username)) {
            return false;
        }
        users.remove(username);
        return true;
    }
    
    public boolean changePassword(String username, String oldPassword, String newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(newPassword, salt);
        users.put(username, new UserCredential(salt, hashedPassword));
        return true;
    }
    
    public static void main(String[] args) {
        Task42 authSystem = new Task42();
        
        // Test Case 1: Register new user
        System.out.println("Test 1 - Register user 'alice':");
        boolean result1 = authSystem.registerUser("alice", "password123");
        System.out.println("Registration successful: " + result1);
        
        // Test Case 2: Authenticate with correct password
        System.out.println("\\nTest 2 - Authenticate alice with correct password:");
        boolean result2 = authSystem.authenticateUser("alice", "password123");
        System.out.println("Authentication successful: " + result2);
        
        // Test Case 3: Authenticate with incorrect password
        System.out.println("\\nTest 3 - Authenticate alice with incorrect password:");
        boolean result3 = authSystem.authenticateUser("alice", "wrongpassword");
        System.out.println("Authentication successful: " + result3);
        
        // Test Case 4: Change password
        System.out.println("\\nTest 4 - Change password for alice:");
        boolean result4 = authSystem.changePassword("alice", "password123", "newpassword456");
        System.out.println("Password change successful: " + result4);
        boolean result4b = authSystem.authenticateUser("alice", "newpassword456");
        System.out.println("Authentication with new password: " + result4b);
        
        // Test Case 5: Register duplicate user and delete user
        System.out.println("\\nTest 5 - Register duplicate user and delete:");
        boolean result5a = authSystem.registerUser("alice", "anotherpassword");
        System.out.println("Duplicate registration blocked: " + !result5a);
        boolean result5b = authSystem.deleteUser("alice");
        System.out.println("User deletion successful: " + result5b);
        boolean result5c = authSystem.authenticateUser("alice", "newpassword456");
        System.out.println("Authentication after deletion: " + result5c);
    }
}
