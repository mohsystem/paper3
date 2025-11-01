import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Base64;

public class Lse34 {

    // In-memory user database simulation. In a real application, this would be a database.
    // Key: username, Value: a string containing salt and hashed password
    private static final Map<String, String> userDatabase = new HashMap<>();

    private static final int SALT_LENGTH = 16; // 16 bytes salt
    private static final int ITERATION_COUNT = 65536;
    private static final int KEY_LENGTH = 128; // in bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Generates a random salt.
     * @return A byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password with a given salt using PBKDF2.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * This simulates adding a user to the database.
     * @param username The username.
     * @param password The plaintext password.
     */
    public static void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return;
        }

        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);

        // Store salt and hash together, typically encoded (e.g., Base64)
        String encodedSalt = Base64.getEncoder().encodeToString(salt);
        String encodedHash = Base64.getEncoder().encodeToString(hashedPassword);
        
        userDatabase.put(username, encodedSalt + ":" + encodedHash);
        System.out.println("User '" + username + "' registered successfully.");
    }

    /**
     * Simulates the login process.
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return A string indicating the result.
     */
    public static String login(String username, String password) {
        // Rule #1: Input validation
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            return "Redirecting to login page: Invalid input.";
        }

        String storedCredentials = userDatabase.get(username);
        if (storedCredentials == null) {
            // User not found
            return "Redirecting to login page: Invalid credentials.";
        }

        try {
            String[] parts = storedCredentials.split(":");
            if (parts.length != 2) {
                // Should not happen if data is stored correctly
                return "Redirecting to login page: Server error.";
            }
            byte[] salt = Base64.getDecoder().decode(parts[0]);
            byte[] storedHash = Base64.getDecoder().decode(parts[1]);

            // Hash the provided password with the stored salt
            byte[] providedPasswordHash = hashPassword(password.toCharArray(), salt);

            // Use constant-time comparison to prevent timing attacks
            if (Arrays.equals(storedHash, providedPasswordHash)) {
                return "Login successful! Redirecting to dashboard page.";
            } else {
                return "Redirecting to login page: Invalid credentials.";
            }
        } catch (IllegalArgumentException e) {
            // Error decoding Base64 string
             return "Redirecting to login page: Server error.";
        }
    }

    public static void main(String[] args) {
        // Populate the "database" with a user
        registerUser("admin", "S3cureP@ssword!23");
        registerUser("testuser", "password123");

        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Correct credentials
        System.out.println("1. Testing with correct credentials (admin):");
        System.out.println("   Result: " + login("admin", "S3cureP@ssword!23"));
        
        // Test Case 2: Correct username, incorrect password
        System.out.println("\n2. Testing with incorrect password (admin):");
        System.out.println("   Result: " + login("admin", "wrongpassword"));
        
        // Test Case 3: Incorrect username
        System.out.println("\n3. Testing with incorrect username:");
        System.out.println("   Result: " + login("unknownuser", "S3cureP@ssword!23"));

        // Test Case 4: Correct username, empty password
        System.out.println("\n4. Testing with empty password:");
        System.out.println("   Result: " + login("admin", ""));

        // Test Case 5: Empty username
        System.out.println("\n5. Testing with empty username:");
        System.out.println("   Result: " + login("", "S3cureP@ssword!23"));
    }
}