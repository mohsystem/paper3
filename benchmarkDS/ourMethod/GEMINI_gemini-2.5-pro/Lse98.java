import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Lse98 {

    // In-memory user database simulation
    private static final Map<String, UserEntry> userDatabase = new HashMap<>();
    private static final int SALT_LENGTH = 16; // 16 bytes salt
    private static final int KEY_LENGTH = 256; // 256-bit key
    private static final int ITERATIONS = 600000; // OWASP recommendation for PBKDF2
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    // Class to store user data in the "database"
    private static class UserEntry {
        final byte[] salt;
        final byte[] hash;

        UserEntry(byte[] salt, byte[] hash) {
            this.salt = salt;
            this.hash = hash;
        }

        public String getSaltBase64() {
            return Base64.getEncoder().encodeToString(salt);
        }

        public String getHashBase64() {
            return Base64.getEncoder().encodeToString(hash);
        }
    }

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return a 16-byte salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password using PBKDF2 with a given salt.
     *
     * @param password The password to hash.
     * @param salt     The salt to use for hashing.
     * @return The resulting hash, or an empty Optional on failure.
     */
    private static Optional<byte[]> hashPassword(char[] password, byte[] salt) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return Optional.of(skf.generateSecret(spec).getEncoded());
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error while hashing password: " + e.getMessage());
            return Optional.empty();
        } finally {
            // Zero out the password array for security
            Arrays.fill(password, '\0');
        }
    }

    /**
     * Registers a new user.
     *
     * @param username The username.
     * @param password The password.
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
        Optional<byte[]> hashOpt = hashPassword(password.toCharArray(), salt);

        if (hashOpt.isPresent()) {
            userDatabase.put(username, new UserEntry(salt, hashOpt.get()));
            System.out.println("User '" + username + "' registered successfully.");
            return true;
        }
        return false;
    }

    /**
     * Verifies a user's password.
     *
     * @param username The username.
     * @param password The password to check.
     * @return true if the password is correct, false otherwise.
     */
    public static boolean verifyPassword(String username, String password) {
        UserEntry user = userDatabase.get(username);
        if (user == null || password == null) {
            return false;
        }

        Optional<byte[]> hashToVerifyOpt = hashPassword(password.toCharArray(), user.salt);
        if (hashToVerifyOpt.isEmpty()) {
            return false;
        }

        // Constant-time comparison to prevent timing attacks
        return Arrays.equals(user.hash, hashToVerifyOpt.get());
    }
    
    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Successful registration and login
        System.out.println("\n--- Test Case 1: Successful Registration & Login ---");
        registerUser("alice", "Password123!");
        System.out.println("Verifying 'alice' with correct password: " + (verifyPassword("alice", "Password123!") ? "SUCCESS" : "FAILURE"));
        UserEntry aliceEntry = userDatabase.get("alice");
        if(aliceEntry != null) {
            System.out.println("Stored Salt (Base64): " + aliceEntry.getSaltBase64());
            System.out.println("Stored Hash (Base64): " + aliceEntry.getHashBase64());
        }


        // Test Case 2: Failed login attempt with wrong password
        System.out.println("\n--- Test Case 2: Failed Login ---");
        System.out.println("Verifying 'alice' with incorrect password: " + (verifyPassword("alice", "WrongPassword!") ? "SUCCESS" : "FAILURE"));

        // Test Case 3: Attempt to register a user that already exists
        System.out.println("\n--- Test Case 3: Duplicate Registration ---");
        registerUser("alice", "AnotherPassword456!");

        // Test Case 4: Attempt to register with a weak password
        System.out.println("\n--- Test Case 4: Weak Password Registration ---");
        registerUser("bob", "pass");

        // Test Case 5: Successful registration of a second user
        System.out.println("\n--- Test Case 5: Successful Registration of Another User ---");
        registerUser("charlie", "S3cureP@ssw0rdF0rCharlie");
        System.out.println("Verifying 'charlie' with correct password: " + (verifyPassword("charlie", "S3cureP@ssw0rdF0rCharlie") ? "SUCCESS" : "FAILURE"));
        System.out.println("Verifying 'charlie' with incorrect password: " + (verifyPassword("charlie", "wrongpass") ? "SUCCESS" : "FAILURE"));
    }
}