import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse99 {

    private static final int SALT_LENGTH = 16; // 16 bytes = 128 bits
    private static final int ITERATION_COUNT = 310000;
    private static final int KEY_LENGTH = 256; // bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    // In a real application, this would be a database.
    private static final Map<String, String[]> userDatabase = new HashMap<>();

    /**
     * Generates a cryptographically secure random salt.
     * @return A byte array containing the salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with a salt.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    public static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * Simulates inserting user credentials into a database.
     * @param username The username.
     * @param salt The salt used for hashing (Base64 encoded).
     * @param hashedPassword The hashed password (Base64 encoded).
     */
    public static void insertIntoDatabase(String username, String salt, String hashedPassword) {
        System.out.println("Simulating database insert for user: " + username);
        // In a real application, you would store username, salt, and hashedPassword
        // in your database.
        userDatabase.put(username, new String[]{salt, hashedPassword});
        System.out.println(" -> Stored Salt: " + salt);
        System.out.println(" -> Stored Hash: " + hashedPassword);
        System.out.println("User '" + username + "' successfully registered.");
    }

    /**
     * Registers a new user.
     * @param username The username.
     * @param password The user's password.
     * @return true if registration is successful, false otherwise.
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty()) {
            System.err.println("Username cannot be empty.");
            return false;
        }
        if (password == null || password.length() < 8) {
            System.err.println("Password must be at least 8 characters long.");
            return false;
        }
        if (userDatabase.containsKey(username)) {
            System.err.println("Username '" + username + "' already exists.");
            return false;
        }

        byte[] salt = generateSalt();
        char[] passwordChars = password.toCharArray();
        byte[] hashedPassword = hashPassword(passwordChars, salt);
        
        // Securely clear the password from memory
        java.util.Arrays.fill(passwordChars, '\0');

        String saltString = Base64.getEncoder().encodeToString(salt);
        String hashString = Base64.getEncoder().encodeToString(hashedPassword);

        insertIntoDatabase(username, saltString, hashString);
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        System.out.println("\n--- Test Case 1: Valid user ---");
        registerUser("alice", "Password123!");

        System.out.println("\n--- Test Case 2: Another valid user ---");
        registerUser("bob", "SecurePass!@#$");

        System.out.println("\n--- Test Case 3: User with short password ---");
        registerUser("charlie", "short");

        System.out.println("\n--- Test Case 4: Duplicate username ---");
        registerUser("alice", "AnotherPassword456");

        System.out.println("\n--- Test Case 5: User with empty username ---");
        registerUser("", "ValidPassword123");

        System.out.println("\n--- Test Cases Finished ---");
    }
}