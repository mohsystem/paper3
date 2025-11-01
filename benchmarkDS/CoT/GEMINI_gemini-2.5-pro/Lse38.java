import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Base64;

public class Lse38 {

    // In-memory map to simulate a user database.
    // In a real application, this would be a database table.
    private static final Map<String, UserData> userDatabase = new HashMap<>();

    // UserData class to store salt and hashed password
    private static class UserData {
        private final byte[] salt;
        private final byte[] hashedPassword;

        public UserData(byte[] salt, byte[] hashedPassword) {
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

    /**
     * Hashes a password with a given salt using SHA-256.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            return md.digest(password.getBytes());
        } catch (NoSuchAlgorithmException e) {
            // This should never happen with SHA-256
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }
    
    /**
     * Generates a cryptographically secure salt.
     * @return A new salt as a byte array.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Simulates user registration by adding a user to the "database".
     * In a real app, this would happen on a registration page.
     * @param username The username to register.
     * @param password The password for the user.
     */
    public static void registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return;
        }
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);
        userDatabase.put(username, new UserData(salt, hashedPassword));
    }

    /**
     * Simulates a login attempt.
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return A string indicating the result of the login attempt.
     */
    public static String login(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return "Invalid credentials. Redirecting to login page.";
        }

        UserData userData = userDatabase.get(username);
        if (userData == null) {
            // User not found. To prevent username enumeration, we can perform a dummy hash.
            // For this example, we return a generic message.
            return "Invalid credentials. Redirecting to login page.";
        }

        byte[] providedPasswordHash = hashPassword(password, userData.getSalt());

        // Use a constant-time comparison to prevent timing attacks
        if (MessageDigest.isEqual(providedPasswordHash, userData.getHashedPassword())) {
            return "Login successful. Redirecting to dashboard.";
        } else {
            return "Invalid credentials. Redirecting to login page.";
        }
    }

    public static void main(String[] args) {
        // Setup: Register a user for testing purposes
        registerUser("admin", "Password123!");
        registerUser("user", "userpass");

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Correct username and password
        System.out.println("Test 1: Correct credentials ('admin', 'Password123!')");
        System.out.println("Result: " + login("admin", "Password123!"));
        System.out.println();

        // Test Case 2: Correct username, incorrect password
        System.out.println("Test 2: Correct username, incorrect password ('admin', 'wrongpass')");
        System.out.println("Result: " + login("admin", "wrongpass"));
        System.out.println();

        // Test Case 3: Incorrect username
        System.out.println("Test 3: Incorrect username ('guest', 'Password123!')");
        System.out.println("Result: " + login("guest", "Password123!"));
        System.out.println();

        // Test Case 4: Correct username, empty password
        System.out.println("Test 4: Correct username, empty password ('user', '')");
        System.out.println("Result: " + login("user", ""));
        System.out.println();

        // Test Case 5: Empty username
        System.out.println("Test 5: Empty username ('', 'userpass')");
        System.out.println("Result: " + login("", "userpass"));
        System.out.println();
    }
}