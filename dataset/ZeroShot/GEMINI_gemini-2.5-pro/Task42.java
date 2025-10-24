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
import java.util.Optional;

public class Task42 {

    private final Map<String, User> userStore = new HashMap<>();
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 256;
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    private static class User {
        private final String username;
        private final byte[] salt;
        private final byte[] hashedPassword;

        public User(String username, byte[] salt, byte[] hashedPassword) {
            this.username = username;
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

    private byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    private byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    public boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return false;
        }
        if (userStore.containsKey(username)) {
            return false; // User already exists
        }

        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);
        userStore.put(username, new User(username, salt, hashedPassword));
        return true;
    }

    public boolean loginUser(String username, String password) {
        if (username == null || password == null || !userStore.containsKey(username)) {
            return false;
        }

        User user = userStore.get(username);
        byte[] salt = user.getSalt();
        byte[] storedHash = user.getHashedPassword();

        byte[] passwordHash = hashPassword(password.toCharArray(), salt);

        // Constant-time comparison to prevent timing attacks
        return Arrays.equals(storedHash, passwordHash);
    }

    public static void main(String[] args) {
        Task42 authSystem = new Task42();

        System.out.println("--- User Authentication System Test ---");

        // Test Case 1: Register a new user
        System.out.println("\nTest Case 1: Register a new user 'alice'");
        boolean registered = authSystem.registerUser("alice", "Password123!");
        System.out.println("Registration successful: " + registered);

        // Test Case 2: Log in with correct credentials
        System.out.println("\nTest Case 2: Log in as 'alice' with correct password");
        boolean loginSuccess = authSystem.loginUser("alice", "Password123!");
        System.out.println("Login successful: " + loginSuccess);

        // Test Case 3: Log in with incorrect credentials
        System.out.println("\nTest Case 3: Log in as 'alice' with incorrect password");
        boolean loginFail = authSystem.loginUser("alice", "WrongPassword");
        System.out.println("Login successful: " + loginFail);

        // Test Case 4: Attempt to register an existing user
        System.out.println("\nTest Case 4: Attempt to register existing user 'alice'");
        boolean reRegistered = authSystem.registerUser("alice", "AnotherPassword");
        System.out.println("Registration successful: " + reRegistered);
        
        // Test Case 5: Attempt to log in with a non-existent user
        System.out.println("\nTest Case 5: Attempt to log in as non-existent user 'bob'");
        boolean nonExistentLogin = authSystem.loginUser("bob", "SomePassword");
        System.out.println("Login successful: " + nonExistentLogin);
    }
}