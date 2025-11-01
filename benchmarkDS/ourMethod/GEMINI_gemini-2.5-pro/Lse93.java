import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.HexFormat;

public class Lse93 {

    // Use a high iteration count for PBKDF2 as recommended by security standards.
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256; // bits
    private static final int SALT_LENGTH = 16; // bytes

    // In a real application, this would be a proper database.
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Hashes a password using PBKDF2 with HMAC-SHA256.
     *
     * @param password The password to hash.
     * @param salt     The salt to use for hashing.
     * @return The hashed password as a byte array.
     */
    public static byte[] hashPassword(final char[] password, final byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    /**
     * Generates a cryptographically strong random salt.
     *
     * @return A new salt as a byte array.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Simulates inserting user data into a database.
     * In a real application, this would involve a database connection and an INSERT statement.
     *
     * @param username       The username.
     * @param storedPassword The salted and hashed password string for storage.
     */
    public static void insertIntoDatabase(String username, String storedPassword) {
        userDatabase.put(username, storedPassword);
        System.out.println("User '" + username + "' registered successfully.");
        // For demonstration, we print the stored value. In production, never log this.
        System.out.println("Stored format (salt:hash): " + storedPassword);
        System.out.println("----------------------------------------");
    }

    /**
     * Registers a new user by hashing their password and storing the result.
     *
     * @param username The username to register.
     * @param password The user's password.
     */
    public static void registerUser(String username, char[] password) {
        if (username == null || username.trim().isEmpty() || password == null || password.length == 0) {
            System.err.println("Username and password cannot be empty.");
            return;
        }

        try {
            byte[] salt = generateSalt();
            byte[] hashedPassword = hashPassword(password, salt);

            // Store salt and hash together. A common format is salt:hash.
            // Using Base64 for a text-safe representation.
            String saltString = Base64.getEncoder().encodeToString(salt);
            String hashString = Base64.getEncoder().encodeToString(hashedPassword);
            String storedPassword = saltString + ":" + hashString;

            insertIntoDatabase(username, storedPassword);

        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error while hashing password: " + e.getMessage());
        } finally {
            // Securely clear the password from memory after use.
            Arrays.fill(password, '\0');
        }
    }


    public static void main(String[] args) {
        System.out.println("--- Running User Registration Test Cases ---");

        // Test Case 1
        registerUser("alice", "Password123!".toCharArray());

        // Test Case 2
        registerUser("bob", "S3cureP@ssw0rd".toCharArray());

        // Test Case 3
        registerUser("charlie", "my-weak-password".toCharArray());
        
        // Test Case 4: Empty password
        registerUser("david", "".toCharArray());

        // Test Case 5: Empty username
        registerUser("", "anotherPassword".toCharArray());
    }
}