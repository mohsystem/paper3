
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.sql.*;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse57 {
    // Secure database connection with prepared statements
    private static final String DB_URL = System.getenv("DB_URL");
    private static final String DB_USER = System.getenv("DB_USER");
    private static final String DB_PASS = System.getenv("DB_PASS");
    
    // Simulated user database for demonstration (in production, use real database)
    private static Map<String, UserRecord> userDatabase = new HashMap<>();
    
    static class UserRecord {
        String email;
        String passwordHash;
        byte[] salt;
        
        UserRecord(String email, String passwordHash, byte[] salt) {
            this.email = email;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    // Generate cryptographically secure salt - Rule#8, Rule#9
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    // Hash password with PBKDF2-HMAC-SHA256 - Rule#6, Rule#15
    private static String hashPassword(String password, byte[] salt) {
        try {
            // Use PBKDF2 with at least 210000 iterations
            PBEKeySpec spec = new PBEKeySpec(
                password.toCharArray(), 
                salt, 
                210000, 
                256
            );
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            spec.clearPassword(); // Clear sensitive data
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Password hashing failed", e);
        }
    }
    
    // Constant-time comparison to prevent timing attacks - Rule#6
    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] aBytes = a.getBytes(StandardCharsets.UTF_8);
        byte[] bBytes = b.getBytes(StandardCharsets.UTF_8);
        return MessageDigest.isEqual(aBytes, bBytes);
    }
    
    // Validate email format - Rule#1, Rule#13
    private static boolean isValidEmail(String email) {
        if (email == null || email.length() > 254) return false;
        // Basic email validation - reject obviously invalid formats
        String emailRegex = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
        return email.matches(emailRegex);
    }
    
    // Verify password meets policy - Rule#15
    private static boolean meetsPasswordPolicy(String password) {
        if (password == null || password.length() < 12) return false;
        // Require at least: 12 chars, 1 upper, 1 lower, 1 digit, 1 special
        boolean hasUpper = password.chars().anyMatch(Character::isUpperCase);
        boolean hasLower = password.chars().anyMatch(Character::isLowerCase);
        boolean hasDigit = password.chars().anyMatch(Character::isDigit);
        boolean hasSpecial = password.chars().anyMatch(c -> "!@#$%^&*()_+-=[]{}|;:,.<>?".indexOf(c) >= 0);
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    // Change email securely - Rule#1, Rule#2, Rule#16
    public static String changeEmail(String sessionUserId, String oldEmail, String newEmail, String confirmPassword) {
        try {
            // Input validation - Rule#13
            if (sessionUserId == null || sessionUserId.trim().isEmpty()) {
                return "Error: Not authenticated";
            }
            
            if (oldEmail == null || oldEmail.trim().isEmpty() || 
                newEmail == null || newEmail.trim().isEmpty() ||
                confirmPassword == null || confirmPassword.isEmpty()) {
                return "Error: All fields required";
            }
            
            // Sanitize inputs - trim whitespace only
            oldEmail = oldEmail.trim();
            newEmail = newEmail.trim();
            
            // Validate email formats - Rule#13
            if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) {
                return "Error: Invalid email format";
            }
            
            // Check email length limits - Rule#14
            if (oldEmail.length() > 254 || newEmail.length() > 254) {
                return "Error: Email too long";
            }
            
            // Prevent same old and new email
            if (oldEmail.equalsIgnoreCase(newEmail)) {
                return "Error: New email must be different";
            }
            
            // Retrieve user record
            UserRecord user = userDatabase.get(sessionUserId);
            if (user == null) {
                return "Error: User not found";
            }
            
            // Verify old email matches current email - Rule#1
            if (!constantTimeEquals(user.email, oldEmail)) {
                return "Error: Old email incorrect";
            }
            
            // Verify password - Rule#6, Rule#15
            String providedHash = hashPassword(confirmPassword, user.salt);
            if (!constantTimeEquals(user.passwordHash, providedHash)) {
                // Clear sensitive data
                providedHash = null;
                return "Error: Password incorrect";
            }
            
            // Clear password hash from memory
            providedHash = null;
            
            // Check if new email already exists
            for (UserRecord record : userDatabase.values()) {
                if (constantTimeEquals(record.email, newEmail)) {
                    return "Error: Email already in use";
                }
            }
            
            // Update email
            user.email = newEmail;
            
            return "Success: Email changed";
            
        } catch (Exception e) {
            // Rule#1, Rule#16 - Generic error, log details securely
            System.err.println("Email change error: " + e.getClass().getName());
            return "Error: Operation failed";
        }
    }
    
    // Helper to create test user
    private static void createUser(String userId, String email, String password) {
        byte[] salt = generateSalt();
        String hash = hashPassword(password, salt);
        userDatabase.put(userId, new UserRecord(email, hash, salt));
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("=== Email Change System Test Cases ===\\n");
        
        // Setup test users
        createUser("user1", "alice@example.com", "SecurePass123!");
        createUser("user2", "bob@example.com", "AnotherPass456#");
        
        // Test 1: Successful email change
        System.out.println("Test 1: Valid email change");
        String result1 = changeEmail("user1", "alice@example.com", "alice.new@example.com", "SecurePass123!");
        System.out.println(result1);
        System.out.println();
        
        // Test 2: Wrong old email
        System.out.println("Test 2: Wrong old email");
        String result2 = changeEmail("user2", "wrong@example.com", "bob.new@example.com", "AnotherPass456#");
        System.out.println(result2);
        System.out.println();
        
        // Test 3: Wrong password
        System.out.println("Test 3: Wrong password");
        String result3 = changeEmail("user2", "bob@example.com", "bob.new@example.com", "WrongPassword!");
        System.out.println(result3);
        System.out.println();
        
        // Test 4: Invalid email format
        System.out.println("Test 4: Invalid email format");
        String result4 = changeEmail("user1", "alice.new@example.com", "invalid-email", "SecurePass123!");
        System.out.println(result4);
        System.out.println();
        
        // Test 5: Empty fields
        System.out.println("Test 5: Empty fields");
        String result5 = changeEmail("user2", "", "bob.final@example.com", "AnotherPass456#");
        System.out.println(result5);
        System.out.println();
    }
}
