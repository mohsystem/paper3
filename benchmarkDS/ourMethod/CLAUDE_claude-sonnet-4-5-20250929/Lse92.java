
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse92 {
    private static final int ITERATIONS = 210000; // PBKDF2 iterations >= 210000 for security
    private static final int KEY_LENGTH = 256; // 256-bit key
    private static final int SALT_LENGTH = 16; // 16-byte salt
    
    /**
     * Registers a user by hashing their password with a unique salt.
     * 
     * @param username The username for registration
     * @param password The password to be hashed
     * @return A string containing the salt and hash, separated by ':'
     */
    public static String registerUser(String username, char[] password) {
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be empty");
        }
        if (password == null || password.length < 8) {
            throw new IllegalArgumentException("Password must be at least 8 characters");
        }
        
        try {
            // Generate a cryptographically secure random salt (unique per user)
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH];
            random.nextBytes(salt);
            
            // Hash the password using PBKDF2-HMAC-SHA256
            KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            
            // Clear the password from memory immediately after use
            java.util.Arrays.fill(password, '\\0');
            
            // Encode salt and hash as Base64 for storage
            String saltEncoded = Base64.getEncoder().encodeToString(salt);
            String hashEncoded = Base64.getEncoder().encodeToString(hash);
            
            return saltEncoded + ":" + hashEncoded;
        } catch (Exception e) {
            throw new RuntimeException("Error during password hashing", e);
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid username and password
        String result1 = registerUser("alice", "SecurePass123!".toCharArray());
        System.out.println("User 'alice' registered. Hash: " + result1.substring(0, 50) + "...");
        
        // Test case 2: Different user with different password
        String result2 = registerUser("bob", "AnotherP@ss456".toCharArray());
        System.out.println("User 'bob' registered. Hash: " + result2.substring(0, 50) + "...");
        
        // Test case 3: Same password for different user should produce different hash
        String result3 = registerUser("charlie", "SecurePass123!".toCharArray());
        System.out.println("User 'charlie' registered. Hash: " + result3.substring(0, 50) + "...");
        System.out.println("Note: Same password produces different hash due to unique salt");
        
        // Test case 4: Minimum password length
        String result4 = registerUser("dave", "MinPass8".toCharArray());
        System.out.println("User 'dave' registered. Hash: " + result4.substring(0, 50) + "...");
        
        // Test case 5: Long username and password
        String result5 = registerUser("elizabeth_longusername", "VeryLongP@ssw0rd!WithSpecialChars".toCharArray());
        System.out.println("User 'elizabeth_longusername' registered. Hash: " + result5.substring(0, 50) + "...");
    }
}
