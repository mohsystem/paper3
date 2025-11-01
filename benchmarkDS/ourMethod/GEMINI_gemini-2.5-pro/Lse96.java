import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse96 {

    // Using PBKDF2 with HMAC-SHA256, as recommended for password hashing.
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    // OWASP recommends 310,000 for SHA256.
    private static final int ITERATIONS = 310000;
    // 256-bit key length for SHA-256
    private static final int KEY_LENGTH = 256;
    // 16-byte (128-bit) salt, as recommended
    private static final int SALT_SIZE = 16;

    // A simple in-memory map to simulate a user database
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Generates a cryptographically secure random salt.
     * @return A byte array containing the salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_SIZE];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with a given salt.
     * @param password The password to hash.
     * @param salt The salt to use for hashing.
     * @return The hashed password as a byte array.
     * @throws NoSuchAlgorithmException If the hashing algorithm is not available.
     * @throws InvalidKeySpecException If the provided key spec is invalid.
     */
    public static byte[] hashPassword(char[] password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
        return factory.generateSecret(spec).getEncoded();
    }

    /**
     * Simulates user registration. Hashes the password with a new salt and returns a combined string for storage.
     * @param username The username for registration.
     * @param password The plaintext password.
     * @return A string in the format "salt:hash" (both Base64 encoded) for storage, or null on error.
     */
    public static String registerUser(String username, String password) {
        try {
            byte[] salt = generateSalt();
            byte[] hashedPassword = hashPassword(password.toCharArray(), salt);

            // Store as Base64(salt):Base64(hash) for portability and to avoid raw byte issues
            String saltStr = Base64.getEncoder().encodeToString(salt);
            String hashStr = Base64.getEncoder().encodeToString(hashedPassword);

            String storedPassword = saltStr + ":" + hashStr;
            // Simulate storing in a database
            userDatabase.put(username, storedPassword);
            return storedPassword;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this error securely without exposing details to the user
            System.err.println("Error during password hashing: " + e.getMessage());
            return null;
        }
    }
    
    /**
     * Verifies a provided password against the stored hash for a given user.
     * @param username The user to verify.
     * @param providedPassword The plaintext password to check.
     * @return true if the password is correct, false otherwise.
     */
    public static boolean verifyPassword(String username, String providedPassword) {
        String storedPassword = userDatabase.get(username);
        if (storedPassword == null) {
            return false; // User not found
        }
        try {
            String[] parts = storedPassword.split(":");
            if (parts.length != 2) {
                return false; // Invalid stored format
            }
            byte[] salt = Base64.getDecoder().decode(parts[0]);
            byte[] originalHash = Base64.getDecoder().decode(parts[1]);

            byte[] newHash = hashPassword(providedPassword.toCharArray(), salt);
            
            // Use a constant-time comparison to prevent timing attacks
            return MessageDigest.isEqual(originalHash, newHash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException | IllegalArgumentException e) {
             System.err.println("Error during password verification: " + e.getMessage());
            return false;
        }
    }


    public static void main(String[] args) {
        System.out.println("--- User Registration and Verification Simulation ---");

        String[][] testCases = {
            {"alice", "Password123!"},
            {"bob", "my$ecretP@ss"},
            {"charlie", "aVeryLongAndComplexPassword-12345"},
            {"dave", "short"},
            {"eve", "another_password"}
        };

        for (String[] testCase : testCases) {
            String username = testCase[0];
            String password = testCase[1];
            
            System.out.println("\n--- Testing for user: " + username + " ---");

            // 1. Registration
            String storedPassword = registerUser(username, password);
            System.out.println("Registering user '" + username + "'.");
            System.out.println("Stored format (Base64(salt):Base64(hash)): " + storedPassword);

            // 2. Verification
            System.out.println("Verifying with correct password...");
            boolean isCorrect = verifyPassword(username, password);
            System.out.println("Verification successful: " + isCorrect);

            System.out.println("Verifying with incorrect password ('wrongpassword')...");
            boolean isIncorrect = verifyPassword(username, "wrongpassword");
            System.out.println("Verification successful: " + isIncorrect);
        }
    }
}