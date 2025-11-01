
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Scanner;

public class Lse92 {
    
    public static class RegistrationResult {
        String username;
        String hashedPassword;
        String salt;
        
        public RegistrationResult(String username, String hashedPassword, String salt) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.salt = salt;
        }
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static RegistrationResult registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be empty");
        }
        if (password == null || password.length() < 8) {
            throw new IllegalArgumentException("Password must be at least 8 characters");
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        
        return new RegistrationResult(username, hashedPassword, salt);
    }
    
    public static void main(String[] args) {
        // Test case 1
        try {
            RegistrationResult result1 = registerUser("john_doe", "SecurePass123");
            System.out.println("Test 1 - Username: " + result1.username);
            System.out.println("Hashed Password: " + result1.hashedPassword);
            System.out.println("Salt: " + result1.salt);
            System.out.println();
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2
        try {
            RegistrationResult result2 = registerUser("alice_smith", "MyP@ssw0rd!");
            System.out.println("Test 2 - Username: " + result2.username);
            System.out.println("Hashed Password: " + result2.hashedPassword);
            System.out.println("Salt: " + result2.salt);
            System.out.println();
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3 - Empty username
        try {
            RegistrationResult result3 = registerUser("", "Password123");
            System.out.println("Test 3 - Username: " + result3.username);
        } catch (Exception e) {
            System.out.println("Test 3 failed (expected): " + e.getMessage());
            System.out.println();
        }
        
        // Test case 4 - Short password
        try {
            RegistrationResult result4 = registerUser("bob_jones", "short");
            System.out.println("Test 4 - Username: " + result4.username);
        } catch (Exception e) {
            System.out.println("Test 4 failed (expected): " + e.getMessage());
            System.out.println();
        }
        
        // Test case 5
        try {
            RegistrationResult result5 = registerUser("emma_wilson", "StrongPassword2024");
            System.out.println("Test 5 - Username: " + result5.username);
            System.out.println("Hashed Password: " + result5.hashedPassword);
            System.out.println("Salt: " + result5.salt);
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }
    }
}
