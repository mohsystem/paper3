import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse35 {

    // A mock database storing: username -> salt:hashedPassword
    private static final Map<String, String> userDatabase = new HashMap<>();

    // Use a strong algorithm like PBKDF2.
    // Parameters should be tuned for your system's performance.
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int ITERATION_COUNT = 65536;
    private static final int KEY_LENGTH = 256;
    private static final int SALT_LENGTH = 16; // 16 bytes is a good standard

    /**
     * Generates a cryptographically secure random salt.
     * @return A byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with the given salt.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * In a real app, this would write to a database.
     */
    private static void registerUser(String username, String password) {
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);
        // Store salt and hash together, encoded as Base64 strings
        String encodedSalt = Base64.getEncoder().encodeToString(salt);
        String encodedHash = Base64.getEncoder().encodeToString(hashedPassword);
        userDatabase.put(username, encodedSalt + ":" + encodedHash);
    }
    
    /**
     * Simulates a login attempt.
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return true if login is successful, false otherwise.
     */
    public static boolean login(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return false;
        }

        String storedData = userDatabase.get(username);
        if (storedData == null) {
            // To prevent timing attacks, we can perform a dummy hash calculation
            // even if the user does not exist.
            hashPassword("dummyPassword".toCharArray(), generateSalt());
            return false;
        }

        String[] parts = storedData.split(":");
        byte[] salt = Base64.getDecoder().decode(parts[0]);
        byte[] storedHash = Base64.getDecoder().decode(parts[1]);

        // Hash the provided password with the stored salt
        byte[] providedPasswordHash = hashPassword(password.toCharArray(), salt);

        // Use a constant-time comparison to prevent timing attacks
        return MessageDigest.isEqual(storedHash, providedPasswordHash);
    }


    public static void main(String[] args) {
        // Setup: Register some users in our mock database
        registerUser("admin", "S3cureP@ssword123!");
        registerUser("user1", "myWeakPassword");

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Successful login for admin
        boolean test1 = login("admin", "S3cureP@ssword123!");
        System.out.println("Test 1 (Correct admin password): " + (test1 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page"));

        // Test Case 2: Failed login for admin (wrong password)
        boolean test2 = login("admin", "wrongpassword");
        System.out.println("Test 2 (Incorrect admin password): " + (test2 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page"));

        // Test Case 3: Failed login for non-existent user
        boolean test3 = login("nonexistentuser", "somepassword");
        System.out.println("Test 3 (Non-existent user): " + (test3 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page"));

        // Test Case 4: Successful login for another user
        boolean test4 = login("user1", "myWeakPassword");
        System.out.println("Test 4 (Correct user1 password): " + (test4 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page"));

        // Test Case 5: Failed login with empty credentials
        boolean test5 = login("", "S3cureP@ssword123!");
        System.out.println("Test 5 (Empty username): " + (test5 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page"));
    }
}