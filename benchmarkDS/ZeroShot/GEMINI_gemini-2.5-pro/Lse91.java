import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.Base64;

public class Lse91 {

    // In-memory map to simulate a user database.
    // In a real application, use a persistent database (e.g., PostgreSQL, MySQL).
    // Key: username, Value: salt:hashedPassword
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Generates a cryptographically strong random salt.
     * @return A byte array containing the salt.
     */
    private static byte[] getSalt() throws NoSuchAlgorithmException {
        SecureRandom sr = SecureRandom.getInstanceStrong();
        byte[] salt = new byte[16];
        sr.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password with a given salt using SHA-256.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a hex string.
     */
    private static String hashPassword(String password, byte[] salt) {
        String hashedPassword = null;
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] bytes = md.digest(password.getBytes());
            StringBuilder sb = new StringBuilder();
            for (byte b : bytes) {
                sb.append(String.format("%02x", b));
            }
            hashedPassword = sb.toString();
        } catch (NoSuchAlgorithmException e) {
            // This should not happen if SHA-256 is a valid algorithm
            e.printStackTrace();
        }
        return hashedPassword;
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * @param username The username for the new account.
     * @param password The plain-text password for the new account.
     * @return true if registration is successful, false otherwise.
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return false;
        }
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }

        try {
            byte[] salt = getSalt();
            String hashedPassword = hashPassword(password, salt);
            if (hashedPassword == null) {
                 System.out.println("Registration failed: Could not hash password.");
                 return false;
            }
            
            // Store the salt and hashed password together.
            // A common format is salt:hash
            String storedValue = Base64.getEncoder().encodeToString(salt) + ":" + hashedPassword;
            userDatabase.put(username, storedValue);
            System.out.println("User '" + username + "' registered successfully.");
            return true;
        } catch (NoSuchAlgorithmException e) {
            System.err.println("FATAL: Could not generate salt. " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases for User Registration ---");

        // Test Case 1: Successful registration
        registerUser("alice", "Password123!");

        // Test Case 2: Successful registration with a different user
        registerUser("bob", "SecurePass@2023");

        // Test Case 3: Attempt to register a duplicate username
        registerUser("alice", "AnotherPassword");

        // Test Case 4: Registration with an empty password
        registerUser("charlie", "");

        // Test Case 5: Successful registration
        registerUser("david", "MyP@ssw0rd");

        System.out.println("\n--- Final Simulated Database State ---");
        // Note: In a real application, NEVER print sensitive data like this.
        // This is for demonstration purposes only.
        for (Map.Entry<String, String> entry : userDatabase.entrySet()) {
            System.out.println("Username: " + entry.getKey() + ", Stored Value: " + entry.getValue());
        }
    }
}