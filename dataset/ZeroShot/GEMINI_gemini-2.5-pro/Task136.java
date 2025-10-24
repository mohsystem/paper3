import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Task136 {

    private static final int SALT_LENGTH = 16;
    private static final Map<String, User> userDatabase = new HashMap<>();

    private static class User {
        private final byte[] salt;
        private final byte[] hashedPassword;

        public User(byte[] salt, byte[] hashedPassword) {
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

    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    private static Optional<byte[]> hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return Optional.of(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            System.err.println("SHA-256 algorithm not found.");
            return Optional.empty();
        }
    }

    public static boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.err.println("Username and password cannot be empty.");
            return false;
        }
        if (userDatabase.containsKey(username)) {
            System.err.println("User '" + username + "' already exists.");
            return false;
        }

        byte[] salt = generateSalt();
        Optional<byte[]> hashedPasswordOpt = hashPassword(password, salt);
        
        if (hashedPasswordOpt.isPresent()) {
            userDatabase.put(username, new User(salt, hashedPasswordOpt.get()));
            return true;
        }
        return false;
    }

    public static boolean authenticateUser(String username, String password) {
        if (username == null || password == null || !userDatabase.containsKey(username)) {
            // Generic message to prevent username enumeration
            return false; 
        }

        User user = userDatabase.get(username);
        byte[] salt = user.getSalt();
        byte[] storedHash = user.getHashedPassword();

        Optional<byte[]> inputHashOpt = hashPassword(password, salt);

        if (inputHashOpt.isPresent()) {
            // Use constant-time comparison to prevent timing attacks
            return MessageDigest.isEqual(storedHash, inputHashOpt.get());
        }
        return false;
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases for User Authentication...");

        // Test Case 1: Register a new user
        System.out.println("\n--- Test Case 1: Successful Registration ---");
        boolean regSuccess = registerUser("alice", "Password123!");
        System.out.println("Registration for 'alice': " + (regSuccess ? "SUCCESS" : "FAIL"));

        // Test Case 2: Authenticate with correct password
        System.out.println("\n--- Test Case 2: Successful Authentication ---");
        boolean authSuccess = authenticateUser("alice", "Password123!");
        System.out.println("Authentication for 'alice' with correct password: " + (authSuccess ? "SUCCESS" : "FAIL"));

        // Test Case 3: Authenticate with incorrect password
        System.out.println("\n--- Test Case 3: Failed Authentication (Wrong Password) ---");
        boolean authFailWrongPass = authenticateUser("alice", "WrongPassword");
        System.out.println("Authentication for 'alice' with wrong password: " + (!authFailWrongPass ? "SUCCESS (Correctly Failed)" : "FAIL"));
        
        // Test Case 4: Authenticate a non-existent user
        System.out.println("\n--- Test Case 4: Failed Authentication (Non-Existent User) ---");
        boolean authFailNonExistent = authenticateUser("bob", "Password123!");
        System.out.println("Authentication for non-existent 'bob': " + (!authFailNonExistent ? "SUCCESS (Correctly Failed)" : "FAIL"));
        
        // Test Case 5: Registering an existing user
        System.out.println("\n--- Test Case 5: Failed Registration (User Exists) ---");
        boolean regFailExists = registerUser("alice", "AnotherPassword");
        System.out.println("Re-registration for 'alice': " + (!regFailExists ? "SUCCESS (Correctly Failed)" : "FAIL"));
        
        // Bonus Test Case: Register another user with the same password
        System.out.println("\n--- Bonus Test Case: Salting Verification ---");
        registerUser("charlie", "Password123!");
        User alice = userDatabase.get("alice");
        User charlie = userDatabase.get("charlie");
        boolean hashesDiffer = !Arrays.equals(alice.getHashedPassword(), charlie.getHashedPassword());
        System.out.println("Alice's and Charlie's (same password) hashes are different due to salting: " + (hashesDiffer ? "SUCCESS" : "FAIL"));
    }
}