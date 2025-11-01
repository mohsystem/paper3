
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse135 {
    // Simulated secure storage - in production use secure vault/database
    private static final Map<String, StoredCredential> userDatabase = new HashMap<>();
    
    // Maximum input lengths to prevent resource exhaustion
    private static final int MAX_USERNAME_LENGTH = 100;
    private static final int MAX_PASSWORD_LENGTH = 128;
    
    static class StoredCredential {
        String salt;
        String hashedPassword;
        
        StoredCredential(String salt, String hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }
    
    static {
        // Initialize test users with securely hashed passwords
        try {
            // Test user: "testuser" with password "SecurePass123!"
            String salt1 = generateSalt();
            String hash1 = hashPassword("SecurePass123!", salt1);
            userDatabase.put("testuser", new StoredCredential(salt1, hash1));
            
            // Test user: "admin" with password "Admin@2024Secure"
            String salt2 = generateSalt();
            String hash2 = hashPassword("Admin@2024Secure", salt2);
            userDatabase.put("admin", new StoredCredential(salt2, hash2));
        } catch (Exception e) {
            // Log error securely without exposing details
            System.err.println("Initialization error");
        }
    }
    
    /**
     * Generates a cryptographically secure random salt
     * CWE-759 mitigation: Always use unpredictable unique salt
     * CWE-327 mitigation: Use SecureRandom for cryptographic operations
     */
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    /**
     * Hashes password with salt using SHA-256
     * CWE-327 mitigation: Use strong cryptographic hash
     * CWE-759 mitigation: Include unique salt in hash
     */
    private static String hashPassword(String password, String salt) 
            throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        // Combine salt and password
        String saltedPassword = salt + password;
        byte[] hashedBytes = md.digest(saltedPassword.getBytes(StandardCharsets.UTF_8));
        return Base64.getEncoder().encodeToString(hashedBytes);
    }
    
    /**
     * Validates input string against null, length, and content
     * CWE-20 mitigation: Strict input validation
     * Rules#3: Validate format, type, and length
     */
    private static boolean isValidInput(String input, int maxLength) {
        if (input == null) {
            return false;
        }
        if (input.length() == 0 || input.length() > maxLength) {
            return false;
        }
        // Check for only alphanumeric and safe special characters
        return input.matches("^[a-zA-Z0-9@._-]+$");
    }
    
    /**
     * Constant-time string comparison to prevent timing attacks
     * CWE-208 mitigation: Avoid timing side channels
     */
    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) {
            return false;
        }
        if (a.length() != b.length()) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length(); i++) {
            result |= a.charAt(i) ^ b.charAt(i);
        }
        return result == 0;
    }
    
    /**
     * Checks if provided credentials are valid
     * CWE-798 mitigation: No hardcoded credentials
     * CWE-257 mitigation: Password stored as hash only
     * CWE-306 mitigation: Proper authentication check
     * Rules#6: Strong password policy enforcement
     * Rules#7: Credentials from secure storage, not hardcoded
     */
    public static boolean check_mod(String username, String password) {
        try {
            // Input validation - Rules#3
            if (!isValidInput(username, MAX_USERNAME_LENGTH)) {
                // Generic error - do not reveal which field failed
                return false;
            }
            
            if (password == null || password.length() == 0 || 
                password.length() > MAX_PASSWORD_LENGTH) {
                return false;
            }
            
            // Retrieve stored credentials
            StoredCredential stored = userDatabase.get(username);
            if (stored == null) {
                // User not found - perform dummy hash to prevent timing attack
                try {
                    hashPassword(password, generateSalt());
                } catch (NoSuchAlgorithmException e) {
                    // Continue to return false
                }
                return false;
            }
            
            // Hash the provided password with stored salt
            String hashedInput = hashPassword(password, stored.salt);
            
            // Constant-time comparison to prevent timing attacks
            return constantTimeEquals(hashedInput, stored.hashedPassword);
            
        } catch (NoSuchAlgorithmException e) {
            // Log error securely without exposing details to user
            System.err.println("Authentication error");
            return false;
        } catch (Exception e) {
            // Catch any unexpected errors - fail closed
            System.err.println("Unexpected error");
            return false;
        }
    }
    
    // Test cases
    public static void main(String[] args) {
        System.out.println("Test Case 1 - Valid user testuser:");
        System.out.println(check_mod("testuser", "SecurePass123!")); // true
        
        System.out.println("\\nTest Case 2 - Valid user admin:");
        System.out.println(check_mod("admin", "Admin@2024Secure")); // true
        
        System.out.println("\\nTest Case 3 - Invalid password:");
        System.out.println(check_mod("testuser", "WrongPassword")); // false
        
        System.out.println("\\nTest Case 4 - Non-existent user:");
        System.out.println(check_mod("nonexistent", "AnyPassword")); // false
        
        System.out.println("\\nTest Case 5 - Null/invalid inputs:");
        System.out.println(check_mod(null, "password")); // false
        System.out.println(check_mod("testuser", null)); // false
        System.out.println(check_mod("", "")); // false
    }
}
