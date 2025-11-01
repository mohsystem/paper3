
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse135 {
    // Simulated secure credential storage with hashed passwords
    private static final Map<String, String> CREDENTIALS = new HashMap<>();
    
    static {
        // In production, use bcrypt/scrypt/Argon2 and store in secure database
        CREDENTIALS.put("admin", hashPassword("Admin@123"));
        CREDENTIALS.put("user1", hashPassword("User@456"));
        CREDENTIALS.put("moderator", hashPassword("Mod@789"));
    }
    
    // Secure password hashing (SHA-256 for demo; use bcrypt in production)
    private static String hashPassword(String password) {
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
            throw new RuntimeException("Hashing algorithm not available", e);
        }
    }
    
    // Constant-time string comparison to prevent timing attacks
    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] aBytes = a.getBytes();
        byte[] bBytes = b.getBytes();
        return MessageDigest.isEqual(aBytes, bBytes);
    }
    
    // Secure credential validation
    public static boolean check_mod(String username, String password) {
        // Input validation
        if (username == null || password == null || 
            username.trim().isEmpty() || password.trim().isEmpty()) {
            return false;
        }
        
        // Sanitize username (prevent injection)
        username = username.trim().replaceAll("[^a-zA-Z0-9_]", "");
        
        // Check if username exists
        if (!CREDENTIALS.containsKey(username)) {
            // Perform dummy hash to prevent timing attacks
            hashPassword(password);
            return false;
        }
        
        // Hash the provided password and compare
        String hashedPassword = hashPassword(password);
        String storedHash = CREDENTIALS.get(username);
        
        // Use constant-time comparison
        return constantTimeEquals(hashedPassword, storedHash);
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        System.out.println("=== Authentication Test Cases ===\\n");
        
        // Test case 1: Valid credentials
        System.out.println("Test 1 - Valid admin credentials:");
        System.out.println("Result: " + check_mod("admin", "Admin@123"));
        
        // Test case 2: Valid user credentials
        System.out.println("\\nTest 2 - Valid user1 credentials:");
        System.out.println("Result: " + check_mod("user1", "User@456"));
        
        // Test case 3: Invalid password
        System.out.println("\\nTest 3 - Invalid password:");
        System.out.println("Result: " + check_mod("admin", "wrongpassword"));
        
        // Test case 4: Non-existent user
        System.out.println("\\nTest 4 - Non-existent user:");
        System.out.println("Result: " + check_mod("hacker", "password123"));
        
        // Test case 5: Null/empty inputs
        System.out.println("\\nTest 5 - Empty credentials:");
        System.out.println("Result: " + check_mod("", ""));
    }
}
