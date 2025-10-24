
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task42 {
    private static class UserCredentials {
        String passwordHash;
        String salt;
        
        UserCredentials(String passwordHash, String salt) {
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    private Map<String, UserCredentials> userDatabase;
    
    public Task42() {
        this.userDatabase = new HashMap<>();
    }
    
    private String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    private String hashPassword(String password, String salt) {
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
        if (username == null || username.trim().isEmpty() || 
            password == null || password.length() < 8) {
            return false;
        }
        
        if (userDatabase.containsKey(username)) {
            return false;
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        userDatabase.put(username, new UserCredentials(passwordHash, salt));
        return true;
    }
    
    public boolean authenticateUser(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        UserCredentials credentials = userDatabase.get(username);
        if (credentials == null) {
            return false;
        }
        
        String hashedInput = hashPassword(password, credentials.salt);
        return hashedInput.equals(credentials.passwordHash);
    }
    
    public boolean changePassword(String username, String oldPassword, String newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        
        if (newPassword == null || newPassword.length() < 8) {
            return false;
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(newPassword, salt);
        userDatabase.put(username, new UserCredentials(passwordHash, salt));
        return true;
    }
    
    public static void main(String[] args) {
        Task42 authSystem = new Task42();
        
        // Test Case 1: Register new user
        System.out.println("Test 1 - Register user 'alice': " + 
            authSystem.registerUser("alice", "SecurePass123"));
        
        // Test Case 2: Authenticate valid user
        System.out.println("Test 2 - Login alice with correct password: " + 
            authSystem.authenticateUser("alice", "SecurePass123"));
        
        // Test Case 3: Authenticate with wrong password
        System.out.println("Test 3 - Login alice with wrong password: " + 
            authSystem.authenticateUser("alice", "WrongPass123"));
        
        // Test Case 4: Register duplicate username
        System.out.println("Test 4 - Register duplicate user 'alice': " + 
            authSystem.registerUser("alice", "AnotherPass123"));
        
        // Test Case 5: Change password
        authSystem.registerUser("bob", "OldPassword123");
        System.out.println("Test 5 - Change bob's password: " + 
            authSystem.changePassword("bob", "OldPassword123", "NewPassword123"));
        System.out.println("Test 5 - Login bob with new password: " + 
            authSystem.authenticateUser("bob", "NewPassword123"));
    }
}
