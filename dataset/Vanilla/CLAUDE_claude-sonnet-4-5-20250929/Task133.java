
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Task133 {
    private Map<String, User> users;
    private Map<String, ResetToken> resetTokens;
    
    static class User {
        String username;
        String email;
        String passwordHash;
        
        public User(String username, String email, String passwordHash) {
            this.username = username;
            this.email = email;
            this.passwordHash = passwordHash;
        }
    }
    
    static class ResetToken {
        String token;
        String username;
        long expiryTime;
        
        public ResetToken(String token, String username, long expiryTime) {
            this.token = token;
            this.username = username;
            this.expiryTime = expiryTime;
        }
        
        public boolean isValid() {
            return System.currentTimeMillis() < expiryTime;
        }
    }
    
    public Task133() {
        this.users = new HashMap<>();
        this.resetTokens = new HashMap<>();
    }
    
    public boolean registerUser(String username, String email, String password) {
        if (users.containsKey(username)) {
            return false;
        }
        String passwordHash = hashPassword(password);
        users.put(username, new User(username, email, passwordHash));
        return true;
    }
    
    public String requestPasswordReset(String username) {
        if (!users.containsKey(username)) {
            return null;
        }
        
        String token = generateToken(username);
        long expiryTime = System.currentTimeMillis() + 3600000; // 1 hour
        resetTokens.put(token, new ResetToken(token, username, expiryTime));
        return token;
    }
    
    public boolean resetPassword(String token, String newPassword) {
        if (!resetTokens.containsKey(token)) {
            return false;
        }
        
        ResetToken resetToken = resetTokens.get(token);
        if (!resetToken.isValid()) {
            resetTokens.remove(token);
            return false;
        }
        
        User user = users.get(resetToken.username);
        if (user == null) {
            return false;
        }
        
        user.passwordHash = hashPassword(newPassword);
        resetTokens.remove(token);
        return true;
    }
    
    public boolean verifyPassword(String username, String password) {
        if (!users.containsKey(username)) {
            return false;
        }
        User user = users.get(username);
        return user.passwordHash.equals(hashPassword(password));
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
            return password;
        }
    }
    
    private String generateToken(String username) {
        String data = username + System.currentTimeMillis() + Math.random();
        return hashPassword(data).substring(0, 32);
    }
    
    public static void main(String[] args) {
        Task133 passwordReset = new Task133();
        
        // Test Case 1: Register a user and verify password
        System.out.println("Test Case 1: Register user");
        boolean registered = passwordReset.registerUser("john_doe", "john@example.com", "password123");
        System.out.println("User registered: " + registered);
        System.out.println("Password verified: " + passwordReset.verifyPassword("john_doe", "password123"));
        System.out.println();
        
        // Test Case 2: Request password reset
        System.out.println("Test Case 2: Request password reset");
        String token = passwordReset.requestPasswordReset("john_doe");
        System.out.println("Reset token generated: " + (token != null));
        System.out.println();
        
        // Test Case 3: Reset password with valid token
        System.out.println("Test Case 3: Reset password with valid token");
        boolean resetSuccess = passwordReset.resetPassword(token, "newPassword456");
        System.out.println("Password reset successful: " + resetSuccess);
        System.out.println("Old password verified: " + passwordReset.verifyPassword("john_doe", "password123"));
        System.out.println("New password verified: " + passwordReset.verifyPassword("john_doe", "newPassword456"));
        System.out.println();
        
        // Test Case 4: Try to reset with invalid token
        System.out.println("Test Case 4: Reset with invalid token");
        boolean invalidReset = passwordReset.resetPassword("invalidtoken123", "anotherPassword");
        System.out.println("Reset with invalid token: " + invalidReset);
        System.out.println();
        
        // Test Case 5: Request reset for non-existent user
        System.out.println("Test Case 5: Request reset for non-existent user");
        String invalidToken = passwordReset.requestPasswordReset("non_existent_user");
        System.out.println("Token for non-existent user: " + (invalidToken != null));
    }
}
