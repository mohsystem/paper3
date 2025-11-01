import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse37 {

    // Simulates a database table. In a real application, this would be a database.
    // We store the user, their salt, and the salted+hashed password.
    private static final Map<String, UserCredentials> userDatabase = new HashMap<>();

    // A simple record to hold salt and hashed password for a user.
    private record UserCredentials(byte[] salt, byte[] hashedPassword) {}

    /**
     * Hashes a password using a given salt and SHA-256.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password.
     * @throws NoSuchAlgorithmException if the hashing algorithm is not available.
     */
    private static byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        return md.digest(password.getBytes(StandardCharsets.UTF_8));
    }

    /**
     * Securely checks a user's credentials.
     * @param username The username to check.
     * @param password The password to check.
     * @return A string indicating the result: "success", "cant_login", or "internal_server_error".
     */
    public static String login(String username, String password) {
        try {
            if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
                return "cant_login";
            }

            UserCredentials credentials = userDatabase.get(username);
            if (credentials == null) {
                // To prevent timing attacks that could reveal if a username exists,
                // we perform a dummy hash and comparison.
                byte[] dummySalt = new byte[16];
                byte[] dummyHash = new byte[32];
                byte[] attemptedHash = hashPassword(password, dummySalt);
                MessageDigest.isEqual(attemptedHash, dummyHash); // Constant-time comparison
                return "cant_login";
            }

            // Hash the provided password with the user's stored salt
            byte[] attemptedHash = hashPassword(password, credentials.salt());

            // Use a constant-time comparison to prevent timing attacks
            if (MessageDigest.isEqual(attemptedHash, credentials.hashedPassword())) {
                return "success";
            } else {
                return "cant_login";
            }
        } catch (NoSuchAlgorithmException e) {
            // Log the exception in a real application
            System.err.println("Error: Hashing algorithm not found. " + e.getMessage());
            return "internal_server_error";
        } catch (Exception e) {
            // Catch any other unexpected errors
            System.err.println("An unexpected error occurred: " + e.getMessage());
            return "internal_server_error";
        }
    }

    /**
     * Helper method to pre-populate the database for testing.
     * In a real app, this would be part of a user registration process.
     */
    private static void setupTestDatabase() {
        try {
            String testUser = "alice";
            String testPass = "password123";

            // Generate a cryptographically secure random salt
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);

            // Hash the password with the salt
            byte[] hashedPassword = hashPassword(testPass, salt);

            // Store the credentials
            userDatabase.put(testUser, new UserCredentials(salt, hashedPassword));

            System.out.println("Test database setup for user: " + testUser);
            System.out.println("Salt (Base64): " + Base64.getEncoder().encodeToString(salt));
            System.out.println("Hashed Password (Base64): " + Base64.getEncoder().encodeToString(hashedPassword));
            System.out.println("------------------------------------");

        } catch (NoSuchAlgorithmException e) {
            System.err.println("Failed to setup test database: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        setupTestDatabase();

        // Test Cases
        System.out.println("Test Case 1: Correct credentials");
        System.out.println("Login for 'alice' with 'password123': " + login("alice", "password123")); // Expected: success

        System.out.println("\nTest Case 2: Correct username, incorrect password");
        System.out.println("Login for 'alice' with 'wrongpassword': " + login("alice", "wrongpassword")); // Expected: cant_login

        System.out.println("\nTest Case 3: Incorrect username");
        System.out.println("Login for 'bob' with 'password123': " + login("bob", "password123")); // Expected: cant_login

        System.out.println("\nTest Case 4: Empty password");
        System.out.println("Login for 'alice' with '': " + login("alice", "")); // Expected: cant_login
        
        System.out.println("\nTest Case 5: Null username");
        System.out.println("Login for null with 'password123': " + login(null, "password123")); // Expected: cant_login
    }
}