import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.HexFormat;

public class Lse91 {

    // In-memory user database (simulated)
    // In a real application, use a proper database (e.g., MySQL, PostgreSQL).
    private static final Map<String, UserData> userDatabase = new HashMap<>();

    // Class to store user's salt and hashed password
    private static class UserData {
        byte[] salt;
        String hashedPassword;

        UserData(byte[] salt, String hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }

    /**
     * Generates a cryptographically strong random salt.
     * @return a byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password using a given salt with SHA-256 algorithm.
     * @param password The plain text password.
     * @param salt The salt to use for hashing.
     * @return The hexadecimal string representation of the hashed password.
     */
    private static String hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPasswordBytes = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return HexFormat.of().formatHex(hashedPasswordBytes);
        } catch (NoSuchAlgorithmException e) {
            // This should never happen for SHA-256
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * @param username The username for the new account.
     * @param password The password for the new account.
     * @return true if registration is successful, false if the user already exists.
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Error: Username and password cannot be empty.");
            return false;
        }
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }

        byte[] salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);

        userDatabase.put(username, new UserData(salt, hashedPassword));
        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases for User Registration ---");

        // Test Case 1: Successful registration
        System.out.println("\n[Test Case 1]");
        registerUser("alice", "Password123!");

        // Test Case 2: Successful registration of another user
        System.out.println("\n[Test Case 2]");
        registerUser("bob", "SecurePass@2023");

        // Test Case 3: Attempting to register a user that already exists
        System.out.println("\n[Test Case 3]");
        registerUser("alice", "AnotherPassword");

        // Test Case 4: Registration with an empty password
        System.out.println("\n[Test Case 4]");
        registerUser("charlie", "");

        // Test Case 5: Registration with a different password for a new user
        System.out.println("\n[Test Case 5]");
        registerUser("dave", "Password123!");

        // Optional: Print database state to verify
        System.out.println("\n--- Final Database State ---");
        userDatabase.forEach((username, data) -> {
            System.out.println("Username: " + username);
            System.out.println("Salt: " + HexFormat.of().formatHex(data.salt));
            System.out.println("Hashed Password: " + data.hashedPassword);
            System.out.println("--------------------------");
        });
    }
}