
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Task82 {
    
    /**
     * Generates a cryptographically secure random salt
     * @param length The length of the salt in bytes
     * @return Base64 encoded salt string
     */
    public static String generateSalt(int length) {
        SecureRandom secureRandom = new SecureRandom();
        byte[] salt = new byte[length];
        secureRandom.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    /**
     * Computes SHA-256 hash of password combined with salt
     * @param password The password to hash
     * @param salt The salt to combine with password
     * @return Base64 encoded hash string
     */
    public static String hashPasswordWithSalt(String password, String salt) {
        try {
            if (password == null || salt == null) {
                throw new IllegalArgumentException("Password and salt cannot be null");
            }
            
            // Combine password and salt
            String combined = password + salt;
            
            // Use SHA-256 for hashing
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hashBytes = digest.digest(combined.getBytes(StandardCharsets.UTF_8));
            
            // Return Base64 encoded hash
            return Base64.getEncoder().encodeToString(hashBytes);
            
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        }
    }
    
    /**
     * Verifies a password against a stored hash
     * @param password The password to verify
     * @param salt The salt used in original hash
     * @param storedHash The stored hash to compare against
     * @return true if password matches, false otherwise
     */
    public static boolean verifyPassword(String password, String salt, String storedHash) {
        String computedHash = hashPasswordWithSalt(password, salt);
        return MessageDigest.isEqual(
            computedHash.getBytes(StandardCharsets.UTF_8),
            storedHash.getBytes(StandardCharsets.UTF_8)
        );
    }
    
    public static void main(String[] args) {
        System.out.println("=== Password Hashing with Salt - Test Cases ===\\n");
        
        // Test Case 1: Basic password hashing
        System.out.println("Test Case 1: Basic password hashing");
        String password1 = "MySecurePassword123!";
        String salt1 = generateSalt(16);
        String hash1 = hashPasswordWithSalt(password1, salt1);
        System.out.println("Password: " + password1);
        System.out.println("Salt: " + salt1);
        System.out.println("Hash: " + hash1);
        System.out.println("Verification: " + verifyPassword(password1, salt1, hash1));
        System.out.println();
        
        // Test Case 2: Different password, same salt
        System.out.println("Test Case 2: Different password, same salt");
        String password2 = "DifferentPassword456#";
        String hash2 = hashPasswordWithSalt(password2, salt1);
        System.out.println("Password: " + password2);
        System.out.println("Salt: " + salt1);
        System.out.println("Hash: " + hash2);
        System.out.println("Hashes are different: " + (!hash1.equals(hash2)));
        System.out.println();
        
        // Test Case 3: Same password, different salt
        System.out.println("Test Case 3: Same password, different salt");
        String salt3 = generateSalt(16);
        String hash3 = hashPasswordWithSalt(password1, salt3);
        System.out.println("Password: " + password1);
        System.out.println("Salt: " + salt3);
        System.out.println("Hash: " + hash3);
        System.out.println("Hashes are different: " + (!hash1.equals(hash3)));
        System.out.println();
        
        // Test Case 4: Complex password with special characters
        System.out.println("Test Case 4: Complex password with special characters");
        String password4 = "P@ssw0rd!#$%^&*()_+-=[]{}|;:',.<>?/~`";
        String salt4 = generateSalt(32);
        String hash4 = hashPasswordWithSalt(password4, salt4);
        System.out.println("Password: " + password4);
        System.out.println("Salt: " + salt4);
        System.out.println("Hash: " + hash4);
        System.out.println("Verification: " + verifyPassword(password4, salt4, hash4));
        System.out.println();
        
        // Test Case 5: Wrong password verification
        System.out.println("Test Case 5: Wrong password verification");
        String wrongPassword = "WrongPassword";
        boolean verificationResult = verifyPassword(wrongPassword, salt1, hash1);
        System.out.println("Correct Password: " + password1);
        System.out.println("Wrong Password: " + wrongPassword);
        System.out.println("Verification with wrong password: " + verificationResult);
        System.out.println();
    }
}
