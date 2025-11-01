import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse92 {

    // In a real application, this would be a secure database.
    private static final Map<String, UserRecord> userDatabase = new HashMap<>();

    // A simple record to store user data (salt and hashed password)
    private static class UserRecord {
        final String salt;
        final String hashedPassword;

        UserRecord(String salt, String hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }

    /**
     * Hashes the password using PBKDF2 with a random salt.
     *
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a Base64 encoded string.
     */
    public static String hashPassword(String password, byte[] salt) {
        // Use a high number of iterations to make brute-forcing harder.
        int iterations = 65536;
        int keyLength = 256; // for SHA-256
        PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, iterations, keyLength);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = skf.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real app, log this error securely.
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return A 16-byte salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Registers a new user.
     *
     * @param username The username.
     * @param password The password.
     * @return A map containing the salt and hashed password for demonstration.
     */
    public static Map<String, String> registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return null;
        }

        // In a real app, check if the username already exists.
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return null;
        }

        byte[] salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        String saltString = Base64.getEncoder().encodeToString(salt);

        // Store the user record (simulated)
        userDatabase.put(username, new UserRecord(saltString, hashedPassword));

        Map<String, String> result = new HashMap<>();
        result.put("username", username);
        result.put("salt", saltString);
        result.put("hashedPassword", hashedPassword);

        System.out.println("User '" + username + "' registered successfully.");
        System.out.println("  - Salt (Base64): " + saltString);
        System.out.println("  - Hashed Password (Base64): " + hashedPassword);
        System.out.println("------------------------------------");


        return result;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("--- Java Registration Test Cases ---");
        registerUser("alice", "Password123!");
        registerUser("bob", "my$ecretP@ss");
        registerUser("charlie", "aVeryLongAndComplexPassword");
        registerUser("dave", "short");
        registerUser("eve", ""); // Test empty password
        System.out.println("Total users in DB: " + userDatabase.size());
    }
}