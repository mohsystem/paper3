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
import java.security.MessageDigest;

public class Lse36 {

    // In-memory map to simulate a user database.
    // In a real application, this would be a database table.
    // The value stored is a string containing "salt:hashedPassword"
    private static final Map<String, String> userDatabase = new HashMap<>();
    private static final int ITERATIONS = 65536; // Number of iterations for PBKDF2
    private static final int KEY_LENGTH = 256; // Key length in bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Generates a random salt.
     * @return a byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with HMAC SHA256.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return a byte array containing the hashed password.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * This is a helper function to populate our simulated database.
     * @param username The username.
     * @param password The plaintext password.
     */
    private static void registerUser(String username, String password) {
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);
        String encodedSalt = Base64.getEncoder().encodeToString(salt);
        String encodedHash = Base64.getEncoder().encodeToString(hashedPassword);
        userDatabase.put(username, encodedSalt + ":" + encodedHash);
    }
    
    /**
     * Securely checks if the provided password hash matches the stored hash.
     * @param a First byte array.
     * @param b Second byte array.
     * @return true if the arrays are equal, false otherwise.
     */
    private static boolean secureEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }


    /**
     * The main login function.
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return true for successful login, false otherwise.
     */
    public static boolean login(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return false;
        }

        String storedCredentials = userDatabase.get(username);
        if (storedCredentials == null) {
            // To prevent timing attacks, we should still perform a dummy hash
            // calculation even if the user does not exist.
            hashPassword(password.toCharArray(), generateSalt()); 
            System.out.println("Login failed: Username does not exist.");
            return false;
        }

        try {
            String[] parts = storedCredentials.split(":");
            if (parts.length != 2) {
                System.out.println("Login failed: Invalid credential format in database.");
                return false;
            }
            byte[] salt = Base64.getDecoder().decode(parts[0]);
            byte[] storedHash = Base64.getDecoder().decode(parts[1]);

            // Hash the provided password with the stored salt
            byte[] providedPasswordHash = hashPassword(password.toCharArray(), salt);

            // Compare the hashes in a secure, constant-time manner
            if (secureEquals(storedHash, providedPasswordHash)) {
                System.out.println("Login successful for " + username + ". Redirecting to home page...");
                return true;
            } else {
                System.out.println("Login failed: Incorrect password.");
                return false;
            }
        } catch (IllegalArgumentException e) {
             System.out.println("Login failed: Error decoding credentials.");
             return false;
        }
    }

    public static void main(String[] args) {
        // Setup: Register a user for testing purposes
        registerUser("admin", "P@ssword123!");
        registerUser("user", "mySecurePass");

        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Successful login
        System.out.println("\n[Test 1] Correct username and password:");
        login("admin", "P@ssword123!");

        // Test Case 2: Incorrect password
        System.out.println("\n[Test 2] Correct username, incorrect password:");
        login("admin", "WrongPassword");

        // Test Case 3: Non-existent username
        System.out.println("\n[Test 3] Non-existent username:");
        login("guest", "SomePassword");
        
        // Test Case 4: Empty password
        System.out.println("\n[Test 4] Correct username, empty password:");
        login("user", "");

        // Test Case 5: Case-sensitive username
        System.out.println("\n[Test 5] Case-sensitive username check (should fail):");
        login("Admin", "P@ssword123!");

        System.out.println("\n--- Test Cases Finished ---");
    }
}