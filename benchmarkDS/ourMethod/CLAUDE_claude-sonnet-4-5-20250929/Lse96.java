
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse96 {
    // Minimum password length requirement for security
    private static final int MIN_PASSWORD_LENGTH = 8;
    // Salt size in bytes (128 bits)
    private static final int SALT_SIZE = 16;
    // PBKDF2 iteration count (minimum secure value)
    private static final int PBKDF2_ITERATIONS = 210000;
    
    /**
     * Generates a cryptographically secure random salt
     * Uses SecureRandom to ensure unpredictability (CWE-330, CWE-759)
     * @return byte array containing the salt
     */
    private static byte[] generateSalt() {
        SecureRandom secureRandom = new SecureRandom();
        byte[] salt = new byte[SALT_SIZE];
        secureRandom.nextBytes(salt);
        return salt;
    }
    
    /**
     * Hashes password using PBKDF2-HMAC-SHA256
     * Uses high iteration count and unique salt per password (CWE-759, CWE-916)
     * @param password plaintext password
     * @param salt unique salt for this password
     * @return hashed password bytes
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            // Using PBKDF2-HMAC-SHA256 for secure password hashing
            javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
                password.toCharArray(), 
                salt, 
                PBKDF2_ITERATIONS, 
                256
            );
            javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return factory.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            throw new RuntimeException("Password hashing failed", e);
        }
    }
    
    /**
     * Validates username input
     * Prevents injection and ensures valid format (CWE-20)
     * @param username input to validate
     * @return true if valid
     */
    private static boolean validateUsername(String username) {
        if (username == null || username.isEmpty()) {
            return false;
        }
        // Username: 3-50 alphanumeric characters and underscore only
        return username.matches("^[a-zA-Z0-9_]{3,50}$");
    }
    
    /**
     * Validates password meets security requirements
     * Enforces strong password policy (CWE-521)
     * @param password input to validate
     * @return true if valid
     */
    private static boolean validatePassword(String password) {
        if (password == null || password.length() < MIN_PASSWORD_LENGTH) {
            return false;
        }
        // Password must contain: uppercase, lowercase, digit, special char
        boolean hasUpper = password.matches(".*[A-Z].*");
        boolean hasLower = password.matches(".*[a-z].*");
        boolean hasDigit = password.matches(".*[0-9].*");
        boolean hasSpecial = password.matches(".*[!@#$%^&*()_+\\\\-=\\\\[\\\\]{};':\\"\\\\\\\\|,.<>\\\\/?].*");
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    /**
     * Registers a new user with secure password storage
     * @param username user's chosen username\n     * @param password user's chosen password
     * @return database storage string (salt + hash in Base64) or error message
     */
    public static String registerUser(String username, String password) {
        // Input validation - reject invalid data early (CWE-20)
        if (!validateUsername(username)) {
            return "Error: Invalid username format";
        }
        
        if (!validatePassword(password)) {
            return "Error: Password must be at least " + MIN_PASSWORD_LENGTH + 
                   " characters with uppercase, lowercase, digit, and special character";
        }
        
        try {
            // Generate cryptographically secure unique salt (CWE-330, CWE-759)
            byte[] salt = generateSalt();
            
            // Hash password with PBKDF2-HMAC-SHA256 (CWE-916)
            byte[] passwordHash = hashPassword(password, salt);
            
            // Concatenate salt and hash for storage
            byte[] saltAndHash = new byte[salt.length + passwordHash.length];
            System.arraycopy(salt, 0, saltAndHash, 0, salt.length);
            System.arraycopy(passwordHash, 0, saltAndHash, salt.length, passwordHash.length);
            
            // Encode to Base64 for storage (prevents binary data issues)
            String storedValue = Base64.getEncoder().encodeToString(saltAndHash);
            
            // In production: store username and storedValue in database
            // Never log the password or hash (CWE-532)
            return storedValue;
            
        } catch (Exception e) {
            // Return generic error, don't expose internal details (CWE-209)
            return "Error: Registration failed";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid registration
        System.out.println("Test 1 - Valid user:");
        String result1 = registerUser("testUser123", "SecurePass123!");
        System.out.println(result1.startsWith("Error") ? result1 : "Success: Hash stored");
        
        // Test case 2: Weak password (no special char)
        System.out.println("\\nTest 2 - Weak password:");
        String result2 = registerUser("user456", "WeakPass123");
        System.out.println(result2);
        
        // Test case 3: Invalid username (too short)
        System.out.println("\\nTest 3 - Invalid username:");
        String result3 = registerUser("ab", "ValidPass123!");
        System.out.println(result3);
        
        // Test case 4: Password too short
        System.out.println("\\nTest 4 - Short password:");
        String result4 = registerUser("validUser", "Sh0rt!");
        System.out.println(result4);
        
        // Test case 5: Valid registration with different credentials
        System.out.println("\\nTest 5 - Another valid user:");
        String result5 = registerUser("anotherUser_99", "AnotherSecure99#");
        System.out.println(result5.startsWith("Error") ? result5 : "Success: Hash stored");
    }
}
