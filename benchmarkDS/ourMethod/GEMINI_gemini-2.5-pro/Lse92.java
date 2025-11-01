import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.util.Arrays;

public class Lse92 {

    private static final int ITERATIONS = 210000;
    private static final int SALT_SIZE = 16; // in bytes
    private static final int KEY_LENGTH = 256; // in bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Hashes a password using PBKDF2 with HMAC-SHA256.
     *
     * @param username The username (for context, not used in hash).
     * @param password The password to hash, as a char array.
     * @return A string containing the salt and hash, encoded in Base64 and separated by a colon.
     */
    public static String registerUser(String username, char[] password) {
        // Rule #7: Use a strong, cryptographically secure random number generator to create salts.
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_SIZE];
        random.nextBytes(salt);

        // Rule #5: Use PBKDF2-HMAC-SHA-256
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            byte[] hash = skf.generateSecret(spec).getEncoded();
            
            // Encode salt and hash to Base64 to create a storable string.
            // Rule #6: Store the salt with the hash.
            String saltStr = Base64.getEncoder().encodeToString(salt);
            String hashStr = Base64.getEncoder().encodeToString(hash);
            
            return saltStr + ":" + hashStr;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this securely and return a generic error.
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        } finally {
            // Rule #16: Clear sensitive data from memory.
            spec.clearPassword();
            // Also clear the original password array passed into this method.
            Arrays.fill(password, '\0');
        }
    }

    public static void main(String[] args) {
        System.out.println("Running registration simulation...");

        // Test cases
        String[][] testUsers = {
            {"alice", "Password123!"},
            {"bob", "my$ecretP@ssw0rd"},
            {"charlie", "an0th3r_sTr0ng_p@ss"},
            {"david", "w3ak"},
            {"eve", "Eve's_Password_is_very_long_and_secure_hopefully"}
        };

        for (String[] user : testUsers) {
            String username = user[0];
            // Using a char array for the password is more secure than a String
            char[] password = user[1].toCharArray(); 
            try {
                String hashedPassword = registerUser(username, password);
                System.out.println("Username: " + username);
                System.out.println("Password: [REDACTED]");
                System.out.println("Stored Hash: " + hashedPassword);
                System.out.println("---");
            } catch (Exception e) {
                System.err.println("Failed to register user " + username + ": " + e.getMessage());
            } finally {
                 // Ensure the password array is cleared even if hashing fails
                Arrays.fill(password, '\0');
            }
        }
    }
}