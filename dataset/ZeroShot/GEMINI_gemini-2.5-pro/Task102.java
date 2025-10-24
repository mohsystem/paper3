import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Task102 {

    // In-memory map to simulate a database
    private static final Map<String, String> userDatabase = new HashMap<>();
    private static final SecureRandom RAND = new SecureRandom();
    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 512;
    private static final String ALGORITHM = "PBKDF2WithHmacSHA512";

    /**
     * Hashes the password using PBKDF2 with a random salt.
     *
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return An Optional containing the hashed password.
     */
    public static Optional<String> hashPassword(char[] password, byte[] salt) {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        Arrays.fill(password, Character.MIN_VALUE); // Clear password from memory
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hash = skf.generateSecret(spec).getEncoded();
            return Optional.of(Base64.getEncoder().encodeToString(hash));
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error while hashing password: " + e.getMessage());
            return Optional.empty();
        } finally {
            spec.clearPassword();
        }
    }

    /**
     * Generates a random salt.
     *
     * @return A byte array containing the salt.
     */
    public static byte[] getNextSalt() {
        byte[] salt = new byte[16];
        RAND.nextBytes(salt);
        return salt;
    }


    /**
     * Accepts username and a new password, hashes the password, and updates the database.
     *
     * @param username The user's username.
     * @param newPassword The user's new password.
     * @return true if the update was successful, false otherwise.
     */
    public static boolean updateUserPassword(String username, String newPassword) {
        if (username == null || username.trim().isEmpty() || newPassword == null || newPassword.isEmpty()) {
            System.err.println("Username and password cannot be null or empty.");
            return false;
        }

        if (!userDatabase.containsKey(username)) {
            System.err.println("User '" + username + "' not found.");
            return false;
        }
        
        byte[] salt = getNextSalt();
        Optional<String> hashed_password = hashPassword(newPassword.toCharArray(), salt);

        if (hashed_password.isPresent()) {
            // In a real DB, salt and hash would be stored in separate columns.
            // Here we store them together for simplicity: "salt:hash"
            String storedValue = Base64.getEncoder().encodeToString(salt) + ":" + hashed_password.get();
            userDatabase.put(username, storedValue);
            System.out.println("Password for user '" + username + "' has been updated successfully.");
            return true;
        } else {
            System.err.println("Failed to hash password for user '" + username + "'.");
            return false;
        }
    }


    public static void main(String[] args) {
        // Initialize the 'database' with some users
        userDatabase.put("alice", "initial_placeholder_hash");
        userDatabase.put("bob", "initial_placeholder_hash");
        userDatabase.put("charlie", "initial_placeholder_hash");
        userDatabase.put("david", "initial_placeholder_hash");
        userDatabase.put("eve", "initial_placeholder_hash");

        System.out.println("--- Initial Database State ---");
        userDatabase.forEach((user, pass) -> System.out.println("User: " + user + ", Stored: " + pass));
        System.out.println("\n--- Running 5 Test Cases for Password Update ---");

        // Test Case 1: Update alice's password
        updateUserPassword("alice", "P@ssword123!");

        // Test Case 2: Update bob's password
        updateUserPassword("bob", "SecurePwd!#$987");

        // Test Case 3: Update a non-existent user
        updateUserPassword("frank", "somePassword");

        // Test Case 4: Update charlie's password
        updateUserPassword("charlie", "another-strong-password-4-charlie");

        // Test Case 5: Update with an empty password
        updateUserPassword("david", "");
        
        System.out.println("\n--- Final Database State ---");
        userDatabase.forEach((user, pass) -> System.out.println("User: " + user + ", Stored: " + pass));
    }
}