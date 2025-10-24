import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class Task122 {

    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256; // bits
    private static final int SALT_LENGTH = 16; // bytes
    private static final int MIN_PASSWORD_LENGTH = 8;

    /**
     * Hashes a password using PBKDF2WithHmacSHA256.
     *
     * @param password The password to hash.
     * @return A formatted string containing the algorithm, iterations, salt, and hash.
     *         Returns null if password is too short.
     */
    public static String hashPassword(char[] password) {
        if (password == null || password.length < MIN_PASSWORD_LENGTH) {
            // In a real application, throw a specific exception.
            System.err.println("Password does not meet the length requirement.");
            return null;
        }

        try {
            SecureRandom random = SecureRandom.getInstanceStrong();
            byte[] salt = new byte[SALT_LENGTH];
            random.nextBytes(salt);

            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            // Securely clear the password from memory after use
            Arrays.fill(password, Character.MIN_VALUE);

            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();

            String saltBase64 = Base64.getEncoder().encodeToString(salt);
            String hashBase64 = Base64.getEncoder().encodeToString(hash);

            return String.format("$pbkdf2-sha256$%d$%s$%s", ITERATIONS, saltBase64, hashBase64);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this error securely and handle it.
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    /**
     * Verifies a password against a stored hash.
     *
     * @param password The password to verify.
     * @param storedHash The stored hash string from hashPassword.
     * @return True if the password is correct, false otherwise.
     */
    public static boolean verifyPassword(char[] password, String storedHash) {
        if (password == null || storedHash == null) {
            return false;
        }

        String[] parts = storedHash.split("\\$");
        if (parts.length != 5) {
            // Not the format we expect
            return false;
        }

        try {
            int iterations = Integer.parseInt(parts[2]);
            byte[] salt = Base64.getDecoder().decode(parts[3]);
            byte[] originalHash = Base64.getDecoder().decode(parts[4]);

            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, KEY_LENGTH);
             // Securely clear the password from memory after use
            Arrays.fill(password, Character.MIN_VALUE);
            
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] comparisonHash = factory.generateSecret(spec).getEncoded();

            // Constant-time comparison to prevent timing attacks
            return MessageDigest.isEqual(originalHash, comparisonHash);
        } catch (NumberFormatException | IllegalArgumentException | NoSuchAlgorithmException | InvalidKeySpecException e) {
            // Malformed hash string or algorithm not available
            // In a real application, log this attempt.
            return false;
        }
    }

    // In-memory user store for demonstration
    private static final Map<String, String> userDatabase = new HashMap<>();

    public static void signUp(String username, char[] password) {
        System.out.println("Attempting to sign up user: " + username);
        String hashedPassword = hashPassword(password.clone());
        if (hashedPassword != null) {
            userDatabase.put(username, hashedPassword);
            System.out.println("User '" + username + "' signed up successfully.");
        } else {
            System.out.println("Signup for user '" + username + "' failed.");
        }
    }

    public static void login(String username, char[] password) {
        System.out.println("Attempting to log in user: " + username);
        String storedHash = userDatabase.get(username);
        if (storedHash == null) {
            System.out.println("Login failed: User not found.");
            return;
        }
        
        boolean success = verifyPassword(password.clone(), storedHash);
        if (success) {
            System.out.println("Login successful for user '" + username + "'.");
        } else {
            System.out.println("Login failed: Incorrect password.");
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Successful signup and login
        System.out.println("--- Test Case 1 ---");
        String user1 = "alice";
        char[] pass1 = "Str0ngP@ssw0rd1".toCharArray();
        signUp(user1, pass1.clone());
        login(user1, pass1.clone());
        System.out.println();

        // Test Case 2: Login with incorrect password
        System.out.println("--- Test Case 2 ---");
        char[] wrongPass1 = "WrongPassword!_".toCharArray();
        login(user1, wrongPass1.clone());
        System.out.println();

        // Test Case 3: Signup with a weak (short) password
        System.out.println("--- Test Case 3 ---");
        String user2 = "bob";
        char[] pass2_weak = "short".toCharArray();
        signUp(user2, pass2_weak.clone());
        System.out.println();

        // Test Case 4: Successful signup for a second user
        System.out.println("--- Test Case 4 ---");
        String user3 = "charlie";
        char[] pass3 = "An0th3rS3curePwd".toCharArray();
        signUp(user3, pass3.clone());
        login(user3, pass3.clone());
        System.out.println();

        // Test Case 5: Attempt to log in a non-existent user
        System.out.println("--- Test Case 5 ---");
        login("david", "SomePassword123".toCharArray());
        System.out.println();
    }
}