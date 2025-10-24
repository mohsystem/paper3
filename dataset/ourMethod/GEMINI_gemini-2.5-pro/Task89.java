import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task89 {

    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256; // in bits
    private static final int SALT_LENGTH = 16; // in bytes
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Hashes a password with a new salt and stores it in a file.
     * The format is "salt:hash" with Base64 encoding.
     *
     * @param filePath The path to the file where the password hash will be stored.
     * @param password The plaintext password to store.
     * @return true if successful, false otherwise.
     */
    public static boolean storePassword(String filePath, String password) {
        try {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH];
            random.nextBytes(salt);

            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();

            Base64.Encoder enc = Base64.getEncoder();
            String storedPassword = enc.encodeToString(salt) + ":" + enc.encodeToString(hash);
            
            Files.write(Paths.get(filePath), storedPassword.getBytes(StandardCharsets.UTF_8));
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException | IOException e) {
            System.err.println("Error storing password: " + e.getMessage());
            return false;
        }
    }

    /**
     * Verifies a given password against the hash stored in a file.
     * This function performs a constant-time comparison of the hashes.
     *
     * @param filePath The path to the file containing the stored password hash.
     * @param password The plaintext password to verify.
     * @return true if the password matches, false otherwise.
     */
    public static boolean verifyPassword(String filePath, String password) {
        try {
            String storedPassword = new String(Files.readAllBytes(Paths.get(filePath)), StandardCharsets.UTF_8);
            String[] parts = storedPassword.split(":");
            if (parts.length != 2) {
                return false; // Invalid format
            }

            Base64.Decoder dec = Base64.getDecoder();
            byte[] salt = dec.decode(parts[0]);
            byte[] hash = dec.decode(parts[1]);

            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] testHash = factory.generateSecret(spec).getEncoded();

            return MessageDigest.isEqual(hash, testHash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException | IOException | IllegalArgumentException e) {
            // Catches file not found, format errors, etc.
            return false;
        }
    }

    public static void main(String[] args) {
        String testFilePath = "password_storage_java.txt";
        String correctPassword = "Password123!@#";

        System.out.println("--- Java Test Cases for Password Verification ---");

        boolean setupSuccess = storePassword(testFilePath, correctPassword);
        if (!setupSuccess) {
            System.out.println("Failed to set up test file. Aborting tests.");
            return;
        }
        System.out.println("Test file setup complete.");

        // Test Case 1: Correct password
        System.out.println("1. Correct password ('" + correctPassword + "'): " + 
            (verifyPassword(testFilePath, correctPassword) ? "PASSED" : "FAILED"));

        // Test Case 2: Incorrect password
        String wrongPassword = "WrongPassword";
        System.out.println("2. Incorrect password ('" + wrongPassword + "'): " + 
            (!verifyPassword(testFilePath, wrongPassword) ? "PASSED" : "FAILED"));
            
        // Test Case 3: Empty password
        System.out.println("3. Empty password (''): " +
            (!verifyPassword(testFilePath, "") ? "PASSED" : "FAILED"));

        // Test Case 4: Case-sensitive password
        String casePassword = "password123!@#";
        System.out.println("4. Case-sensitive check ('" + casePassword + "'): " + 
            (!verifyPassword(testFilePath, casePassword) ? "PASSED" : "FAILED"));

        // Test Case 5: Non-existent file
        System.out.println("5. Non-existent file: " + 
            (!verifyPassword("nonexistentfile.txt", correctPassword) ? "PASSED" : "FAILED"));
        
        // Clean up
        try {
            Files.deleteIfExists(Paths.get(testFilePath));
            System.out.println("Test file cleaned up.");
        } catch (IOException e) {
            System.err.println("Failed to clean up test file.");
        }
    }
}