import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse98 {

    // Simulated in-memory database
    private static final Map<String, String> database = new HashMap<>();

    // Hashing constants
    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 256;
    private static final int SALT_SIZE = 16; // 16 bytes = 128 bits
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Generates a random salt.
     * @return A byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_SIZE];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with a given salt.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a Base64 encoded string.
     */
    private static String hashPassword(String password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        char[] passwordChars = password.toCharArray();
        KeySpec spec = new PBEKeySpec(passwordChars, salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
        byte[] hash = factory.generateSecret(spec).getEncoded();
        return Base64.getEncoder().encodeToString(hash);
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * @param username The username.
     * @param password The plaintext password.
     * @return true if registration is successful, false otherwise.
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.err.println("Error: Username and password cannot be empty.");
            return false;
        }
        if (database.containsKey(username)) {
            System.err.println("Error: Username '" + username + "' already exists.");
            return false;
        }

        try {
            byte[] salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            // Store the salt and hash together. Format: salt:hash
            String storedValue = Base64.getEncoder().encodeToString(salt) + ":" + hashedPassword;
            database.put(username, storedValue);
            System.out.println("User '" + username + "' registered successfully.");
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Critical Error: Could not perform password hashing.");
            e.printStackTrace();
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Registration Test ---");
        
        // Test Case 1: Successful registration
        registerUser("alice", "Password123!");
        
        // Test Case 2: Successful registration
        registerUser("bob", "SecurePass@2023");
        
        // Test Case 3: Attempt with empty password
        registerUser("charlie", "");
        
        // Test Case 4: Attempt to register a duplicate username
        registerUser("alice", "AnotherPassword");
        
        // Test Case 5: Successful registration with complex password
        registerUser("david", "d@v1d_pa$$w0rd");

        System.out.println("\n--- Simulated Database Contents ---");
        database.forEach((user, storedPass) -> 
            System.out.println("Username: " + user + ", Stored Value: " + storedPass)
        );
        System.out.println("---------------------------------\n");
    }
}