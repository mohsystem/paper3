import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Task42 {

    // In-memory storage for user credentials. In a real application, use a database.
    private static final Map<String, UserCredentials> userStore = new HashMap<>();
    private static final int SALT_LENGTH = 16; // 16 bytes salt

    private static class UserCredentials {
        private final byte[] salt;
        private final byte[] hashedPassword;

        public UserCredentials(byte[] salt, byte[] hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }

        public byte[] getSalt() {
            return salt;
        }

        public byte[] getHashedPassword() {
            return hashedPassword;
        }
    }

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
     * Hashes the password with a given salt using SHA-256.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            return md.digest(password.getBytes());
        } catch (NoSuchAlgorithmException e) {
            // This should never happen for SHA-256
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a new user.
     * @param username The username.
     * @param password The password.
     * @return true if registration is successful, false if the user already exists.
     */
    public static boolean registerUser(String username, String password) {
        if (userStore.containsKey(username)) {
            return false; // User already exists
        }
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);
        userStore.put(username, new UserCredentials(salt, hashedPassword));
        return true;
    }

    /**
     * Authenticates a user.
     * @param username The username.
     * @param password The password.
     * @return true if authentication is successful, false otherwise.
     */
    public static boolean loginUser(String username, String password) {
        if (!userStore.containsKey(username)) {
            return false; // User not found
        }
        UserCredentials credentials = userStore.get(username);
        byte[] providedPasswordHash = hashPassword(password, credentials.getSalt());
        
        // Use constant-time comparison to prevent timing attacks
        return MessageDigest.isEqual(credentials.getHashedPassword(), providedPasswordHash);
    }

    public static void main(String[] args) {
        System.out.println("--- Java User Authentication System Test ---");

        // Test Case 1: Successful registration
        System.out.println("Test 1: Registering a new user 'alice'.");
        boolean regSuccess = registerUser("alice", "Password123!");
        System.out.println("Registration successful: " + regSuccess);
        System.out.println();

        // Test Case 2: Successful login
        System.out.println("Test 2: 'alice' attempts to log in with correct password.");
        boolean loginSuccess = loginUser("alice", "Password123!");
        System.out.println("Login successful: " + loginSuccess);
        System.out.println();

        // Test Case 3: Failed login (wrong password)
        System.out.println("Test 3: 'alice' attempts to log in with incorrect password.");
        boolean loginFailWrongPass = loginUser("alice", "WrongPassword!");
        System.out.println("Login successful: " + loginFailWrongPass);
        System.out.println();

        // Test Case 4: Failed login (non-existent user)
        System.out.println("Test 4: Non-existent user 'bob' attempts to log in.");
        boolean loginFailNoUser = loginUser("bob", "SomePassword");
        System.out.println("Login successful: " + loginFailNoUser);
        System.out.println();

        // Test Case 5: Failed registration (duplicate username)
        System.out.println("Test 5: Attempting to register 'alice' again.");
        boolean regFailDuplicate = registerUser("alice", "AnotherPassword");
        System.out.println("Registration successful: " + regFailDuplicate);
        System.out.println();
    }
}