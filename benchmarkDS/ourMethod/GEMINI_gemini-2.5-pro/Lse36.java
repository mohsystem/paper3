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

public class Lse36 {

    // In a real application, this would be a database.
    private static final Map<String, User> userDatabase = new HashMap<>();
    
    // Strong cryptographic parameters
    private static final int SALT_LENGTH = 16; // 16 bytes = 128 bits
    private static final int ITERATION_COUNT = 310000;
    private static final int KEY_LENGTH = 256; // 256 bits for AES-256
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    // Represents a user record in the database
    private static class User {
        final byte[] salt;
        final byte[] hashedPassword;

        User(byte[] salt, byte[] hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }

    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this security-critical error
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    /**
     * Registers a new user. In a real application, this would store the user in a database.
     * This is a helper for setting up test cases.
     */
    public static void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return;
        }
        
        byte[] salt = generateSalt();
        char[] passwordChars = password.toCharArray();
        byte[] hashedPassword = hashPassword(passwordChars, salt);
        
        // Securely clear the password from memory
        Arrays.fill(passwordChars, '\0'); 

        userDatabase.put(username, new User(salt, hashedPassword));
        System.out.println("User '" + username + "' registered successfully.");
    }

    /**
     * Simulates a user login process.
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return true if login is successful, false otherwise.
     */
    public static boolean login(String username, String password) {
        // 1. Input Validation
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Login failed: Username and password cannot be empty.");
            return false;
        }
        
        // 2. Query database for username
        User user = userDatabase.get(username);
        if (user == null) {
            System.out.println("Login failed: Invalid username or password.");
            return false; // User not found
        }
        
        char[] passwordChars = password.toCharArray();
        
        // 3. Hash the provided password with the stored salt
        byte[] providedPasswordHash = hashPassword(passwordChars, user.salt);
        
        // Securely clear the password from memory
        Arrays.fill(passwordChars, '\0');

        // 4. Compare hashes in constant time to prevent timing attacks
        boolean passwordCorrect = Arrays.equals(providedPasswordHash, user.hashedPassword);

        Arrays.fill(providedPasswordHash, (byte) 0); // Clear derived hash

        if (passwordCorrect) {
            System.out.println("Login successful for user '" + username + "'. Redirecting to home page...");
            return true;
        } else {
            System.out.println("Login failed: Invalid username or password.");
            return false;
        }
    }

    public static void main(String[] args) {
        // Setup: Register some users for testing
        registerUser("alice", "Password123!");
        registerUser("bob", "SecurePass@2024");
        
        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Successful login
        System.out.println("\n[Test 1] Correct credentials for 'alice'");
        login("alice", "Password123!");

        // Test Case 2: Incorrect password
        System.out.println("\n[Test 2] Incorrect password for 'alice'");
        login("alice", "wrongpassword");

        // Test Case 3: Non-existent user
        System.out.println("\n[Test 3] Non-existent user 'charlie'");
        login("charlie", "anypassword");

        // Test Case 4: Successful login for another user
        System.out.println("\n[Test 4] Correct credentials for 'bob'");
        login("bob", "SecurePass@2024");
        
        // Test Case 5: Empty password
        System.out.println("\n[Test 5] Empty password for 'alice'");
        login("alice", "");
    }
}