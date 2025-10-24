import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Base64;

public class Task136 {

    private static final int SALT_LENGTH = 16; // in bytes
    private static final int KEY_LENGTH = 256; // in bits
    private static final int ITERATIONS = 400000;
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    private final Map<String, UserCredentials> userDatabase = new HashMap<>();
    private final SecureRandom secureRandom = new SecureRandom();

    private static class UserCredentials {
        final byte[] salt;
        final byte[] hash;

        UserCredentials(byte[] salt, byte[] hash) {
            this.salt = salt;
            this.hash = hash;
        }
    }

    public boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.length() < MIN_PASSWORD_LENGTH) {
            System.err.println("Error: Invalid username or password does not meet policy.");
            return false;
        }
        if (userDatabase.containsKey(username)) {
            System.err.println("Error: Username '" + username + "' already exists.");
            return false;
        }

        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);

        try {
            byte[] hash = hashPassword(password.toCharArray(), salt);
            userDatabase.put(username, new UserCredentials(salt, hash));
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error during password hashing: " + e.getMessage());
            return false;
        }
    }

    public boolean authenticateUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return false;
        }

        UserCredentials credentials = userDatabase.get(username);
        if (credentials == null) {
            return false; // User not found
        }

        try {
            byte[] calculatedHash = hashPassword(password.toCharArray(), credentials.salt);
            return MessageDigest.isEqual(credentials.hash, calculatedHash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error during password verification: " + e.getMessage());
            return false;
        }
    }

    private static byte[] hashPassword(char[] password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        // Clear password from memory after use
        Arrays.fill(password, Character.MIN_VALUE);
        SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
        return skf.generateSecret(spec).getEncoded();
    }

    public static void main(String[] args) {
        Task136 authService = new Task136();

        // Test Case 1: Register a new user
        System.out.println("Test Case 1: Registering user 'alice'.");
        boolean registered = authService.registerUser("alice", "Password123!");
        System.out.println("Registration successful: " + registered);
        System.out.println("--------------------");

        // Test Case 2: Authenticate user with correct password
        System.out.println("Test Case 2: Authenticating 'alice' with correct password.");
        boolean authenticated = authService.authenticateUser("alice", "Password123!");
        System.out.println("Authentication successful: " + authenticated);
        System.out.println("--------------------");

        // Test Case 3: Authenticate user with incorrect password
        System.out.println("Test Case 3: Authenticating 'alice' with incorrect password.");
        authenticated = authService.authenticateUser("alice", "WrongPassword!");
        System.out.println("Authentication successful: " + authenticated);
        System.out.println("--------------------");

        // Test Case 4: Authenticate a non-existent user
        System.out.println("Test Case 4: Authenticating non-existent user 'bob'.");
        authenticated = authService.authenticateUser("bob", "SomePassword");
        System.out.println("Authentication successful: " + authenticated);
        System.out.println("--------------------");

        // Test Case 5: Register another user and authenticate
        System.out.println("Test Case 5: Registering and authenticating 'charlie'.");
        registered = authService.registerUser("charlie", "SecurePass!@#");
        System.out.println("Registration of 'charlie' successful: " + registered);
        authenticated = authService.authenticateUser("charlie", "SecurePass!@#");
        System.out.println("Authentication of 'charlie' successful: " + authenticated);
        System.out.println("--------------------");

        // Test Case 6: Attempt to register with a short password
        System.out.println("Test Case 6: Attempt to register with a short password.");
        registered = authService.registerUser("david", "short");
        System.out.println("Registration successful: " + registered);
        System.out.println("--------------------");

        // Test Case 7: Attempt to register an existing user
        System.out.println("Test Case 7: Attempt to register 'alice' again.");
        registered = authService.registerUser("alice", "AnotherPassword");
        System.out.println("Registration successful: " + registered);
        System.out.println("--------------------");
    }
}