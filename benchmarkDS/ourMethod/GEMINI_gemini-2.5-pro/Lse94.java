import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;

public class Lse94 {

    private static final int ITERATIONS = 310000;
    private static final int KEY_LENGTH = 512; // in bits
    private static final int SALT_SIZE = 16;   // in bytes
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Hashes a password using PBKDF2 with a random salt.
     * @param username The username, for context (not used in hash).
     * @param password The password to hash.
     * @return A string containing the Base64 encoded salt and hash, separated by a colon.
     * @throws NoSuchAlgorithmException if the specified algorithm is not available.
     * @throws InvalidKeySpecException if the key spec is invalid.
     */
    public static String registerUser(String username, String password)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        
        if (password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Password cannot be null or empty.");
        }
        if (username == null || username.isEmpty()) {
            throw new IllegalArgumentException("Username cannot be null or empty.");
        }

        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_SIZE];
        random.nextBytes(salt);

        char[] passwordChars = password.toCharArray();
        PBEKeySpec spec = new PBEKeySpec(passwordChars, salt, ITERATIONS, KEY_LENGTH);
        
        // Securely clear the password from memory after use
        java.util.Arrays.fill(passwordChars, '\0');

        SecretKeyFactory skf = SecretKeyFactory.getInstance(ALGORITHM);
        byte[] hash = skf.generateSecret(spec).getEncoded();
        
        // Store salt and hash in a single string, separated by a colon
        // Using Base64 for safe encoding
        return Base64.getEncoder().encodeToString(salt) + ":" + Base64.getEncoder().encodeToString(hash);
    }

    /**
     * Simulates inserting user credentials into a database.
     * @param username The username.
     * @param storedPassword The salted and hashed password string.
     */
    private static void insertIntoDatabase(String username, String storedPassword) {
        System.out.println("[DB INSERT] Storing credentials for user: " + username);
        System.out.println("  - Username: " + username);
        System.out.println("  - Stored Password (Salt:Hash): " + storedPassword);
        System.out.println("--------------------------------------------------");
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] testUsers = {
            {"admin", "AdminPassword123!"},
            {"user01", "s#cureP@ssw0rd"},
            {"test_user", "password"},
            {"another_user", "12345678"},
            {"john.doe", "My$tr0ngP@ssw0rdF0rJohn"}
        };

        for (String[] user : testUsers) {
            String username = user[0];
            String password = user[1];
            try {
                System.out.println("Processing user: " + username);
                String hashedPassword = registerUser(username, password);
                insertIntoDatabase(username, hashedPassword);
            } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
                System.err.println("Error hashing password for user " + username + ": " + e.getMessage());
                e.printStackTrace();
            } catch (IllegalArgumentException e) {
                 System.err.println("Invalid input for user " + username + ": " + e.getMessage());
            }
        }
    }
}