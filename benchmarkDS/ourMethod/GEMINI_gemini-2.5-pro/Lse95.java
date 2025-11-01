import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse95 {

    // In a real application, this would be a database.
    private static final Map<String, String> userDatabase = new HashMap<>();

    // Use strong, modern parameters for PBKDF2.
    // OWASP recommends at least 310,000 iterations for PBKDF2-HMAC-SHA256.
    private static final int ITERATIONS = 310000;
    private static final int KEY_LENGTH = 256; // bits
    private static final int SALT_LENGTH = 16; // bytes
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Generates a hash of the password using a random salt.
     *
     * @param password The password to hash.
     * @param salt     The salt to use for hashing.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        // It's important to clear the password from memory after use.
        Arrays.fill(password, Character.MIN_VALUE);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    /**
     * Registers a new user.
     *
     * @param username The username.
     * @param password The password.
     * @return true if registration is successful, false otherwise (e.g., user exists).
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Username and password cannot be empty.");
            return false;
        }

        if (userDatabase.containsKey(username)) {
            System.out.println("User '" + username + "' already exists.");
            return false;
        }
        
        // Generate a cryptographically secure random salt.
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);

        try {
            char[] passwordChars = password.toCharArray();
            byte[] hashedPassword = hashPassword(passwordChars, salt);

            // Combine salt and hash for storage. Format: salt + hash
            byte[] combined = new byte[salt.length + hashedPassword.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(hashedPassword, 0, combined, salt.length, hashedPassword.length);

            // Store the Base64 encoded combined salt and hash.
            userDatabase.put(username, Base64.getEncoder().encodeToString(combined));
            System.out.println("User '" + username + "' registered successfully.");
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this error securely.
            System.err.println("Error during password hashing: " + e.getMessage());
            return false;
        }
    }

    /**
     * Verifies a user's login credentials.
     *
     * @param username The username.
     * @param password The password to verify.
     * @return true if the password is correct, false otherwise.
     */
    public static boolean loginUser(String username, String password) {
         if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return false;
        }
        String storedCombined = userDatabase.get(username);
        if (storedCombined == null) {
            System.out.println("Login failed: User '" + username + "' not found.");
            return false; // User not found
        }

        try {
            byte[] combined = Base64.getDecoder().decode(storedCombined);

            // Extract salt and stored hash from the combined value.
            byte[] salt = Arrays.copyOfRange(combined, 0, SALT_LENGTH);
            byte[] storedHash = Arrays.copyOfRange(combined, SALT_LENGTH, combined.length);

            // Hash the provided password with the retrieved salt.
            char[] passwordChars = password.toCharArray();
            byte[] calculatedHash = hashPassword(passwordChars, salt);

            // Compare the hashes in a way that is resistant to timing attacks.
            boolean passwordMatch = java.security.MessageDigest.isEqual(storedHash, calculatedHash);

            if(passwordMatch){
                System.out.println("Login successful for user '" + username + "'.");
            } else {
                System.out.println("Login failed: Invalid password for user '" + username + "'.");
            }
            return passwordMatch;

        } catch (IllegalArgumentException | NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error during login process: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Successful registration
        System.out.println("\n[Test 1] Registering a new user 'alice'.");
        registerUser("alice", "P@ssw0rd123!");

        // Test Case 2: Attempting to register a user that already exists
        System.out.println("\n[Test 2] Attempting to re-register 'alice'.");
        registerUser("alice", "anotherpassword");

        // Test Case 3: Successful login
        System.out.println("\n[Test 3] Alice logs in with the correct password.");
        loginUser("alice", "P@ssw0rd123!");

        // Test Case 4: Failed login (wrong password)
        System.out.println("\n[Test 4] Alice attempts to log in with an incorrect password.");
        loginUser("alice", "wrongpassword");

        // Test Case 5: Failed login (user not found)
        System.out.println("\n[Test 5] Attempting to log in as a non-existent user 'bob'.");
        loginUser("bob", "anypassword");

        System.out.println("\n--- Test Cases Finished ---");
    }
}