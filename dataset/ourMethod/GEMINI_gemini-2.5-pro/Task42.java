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

public class Task42 {

    private static final int SALT_LENGTH_BYTES = 16;
    private static final int HASH_LENGTH_BYTES = 32; // 256 bits
    private static final int ITERATIONS = 210000;
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    private final Map<String, UserCredentials> userStore = new HashMap<>();
    private final SecureRandom secureRandom = new SecureRandom();

    private static class UserCredentials {
        private final byte[] salt;
        private final byte[] hashedPassword;

        UserCredentials(byte[] salt, byte[] hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }

    /**
     * Hashes a password using PBKDF2 with a given salt.
     *
     * @param password The password to hash.
     * @param salt     The salt to use.
     * @return The hashed password.
     * @throws NoSuchAlgorithmException if the hashing algorithm is not available.
     * @throws InvalidKeySpecException  if the key spec is invalid.
     */
    private byte[] hashPassword(char[] password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_LENGTH_BYTES * 8);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
        return factory.generateSecret(spec).getEncoded();
    }

    /**
     * Registers a new user with a username and password.
     *
     * @param username The username.
     * @param password The password.
     * @return true if registration is successful, false if the user already exists.
     */
    public boolean register(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            System.err.println("Username and password cannot be empty.");
            return false;
        }
        if (userStore.containsKey(username)) {
            System.err.println("User '" + username + "' already exists.");
            return false;
        }

        byte[] salt = new byte[SALT_LENGTH_BYTES];
        secureRandom.nextBytes(salt);
        char[] passwordChars = password.toCharArray();
        try {
            byte[] hashedPassword = hashPassword(passwordChars, salt);
            userStore.put(username, new UserCredentials(salt, hashedPassword));
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error during password hashing: " + e.getMessage());
            return false;
        } finally {
            // Securely clear the password from memory
            Arrays.fill(passwordChars, '\0');
        }
    }

    /**
     * Authenticates a user with a username and password.
     *
     * @param username The username.
     * @param password The password.
     * @return true if authentication is successful, false otherwise.
     */
    public boolean login(String username, String password) {
        if (username == null || password == null || !userStore.containsKey(username)) {
            return false;
        }

        UserCredentials credentials = userStore.get(username);
        char[] passwordChars = password.toCharArray();
        try {
            byte[] computedHash = hashPassword(passwordChars, credentials.salt);
            // Use constant-time comparison to prevent timing attacks
            return MessageDigest.isEqual(computedHash, credentials.hashedPassword);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error during password verification: " + e.getMessage());
            return false;
        } finally {
            // Securely clear the password from memory
            Arrays.fill(passwordChars, '\0');
        }
    }

    public static void main(String[] args) {
        Task42 auth = new Task42();

        // Test Case 1: Register a new user
        System.out.println("Test 1: Register 'alice'");
        boolean registered = auth.register("alice", "Password123!");
        System.out.println("Registration successful: " + registered);
        System.out.println("--------------------");

        // Test Case 2: Successful login
        System.out.println("Test 2: Successful login for 'alice'");
        boolean loginSuccess = auth.login("alice", "Password123!");
        System.out.println("Login successful: " + loginSuccess);
        System.out.println("--------------------");

        // Test Case 3: Failed login with wrong password
        System.out.println("Test 3: Failed login for 'alice' (wrong password)");
        boolean loginFailPassword = auth.login("alice", "WrongPassword");
        System.out.println("Login successful: " + loginFailPassword);
        System.out.println("--------------------");

        // Test Case 4: Failed login with non-existent user
        System.out.println("Test 4: Failed login for non-existent user 'bob'");
        boolean loginFailUser = auth.login("bob", "SomePassword");
        System.out.println("Login successful: " + loginFailUser);
        System.out.println("--------------------");
        
        // Test Case 5: Register a second user and login successfully
        System.out.println("Test 5: Register 'bob' and log in");
        boolean registeredBob = auth.register("bob", "anotherSecureP@ssw0rd");
        System.out.println("Registration of 'bob' successful: " + registeredBob);
        boolean loginBobSuccess = auth.login("bob", "anotherSecureP@ssw0rd");
        System.out.println("Login for 'bob' successful: " + loginBobSuccess);
        System.out.println("--------------------");
    }
}