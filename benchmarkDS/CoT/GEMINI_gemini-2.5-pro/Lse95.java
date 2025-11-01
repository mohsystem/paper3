import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class Lse95 {

    // In-memory user database (for demonstration purposes)
    private static final Map<String, UserData> userDatabase = new HashMap<>();
    private static final int SALT_LENGTH = 16;

    // A simple data class to store user's salt and hashed password
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
     * Generates a cryptographically strong random salt.
     * @return A byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
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
            byte[] passwordBytes = password.getBytes(StandardCharsets.UTF_8);
            return md.digest(passwordBytes);
        } catch (NoSuchAlgorithmException e) {
            // This should never happen with SHA-256
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }

    /**
     * Registers a new user.
     * @param username The username.
     * @param password The password.
     * @return true if registration is successful, false if the user already exists.
     */
    public static boolean registerUser(String username, String password) {
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }

        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);
        userDatabase.put(username, new UserData(salt, hashedPassword));

        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    /**
     * Verifies a user's login credentials.
     * @param username The username.
     * @param password The password to verify.
     * @return true if the credentials are correct, false otherwise.
     */
    public static boolean loginUser(String username, String password) {
        UserData userData = userDatabase.get(username);
        if (userData == null) {
            System.out.println("Login failed: User '" + username + "' not found.");
            return false;
        }

        byte[] providedPasswordHash = hashPassword(password, userData.getSalt());
        boolean isPasswordCorrect = Arrays.equals(providedPasswordHash, userData.getHashedPassword());

        if (isPasswordCorrect) {
            System.out.println("Login successful for user '" + username + "'.");
        } else {
            System.out.println("Login failed: Incorrect password for user '" + username + "'.");
        }
        return isPasswordCorrect;
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Register a new user
        System.out.println("\n[Test Case 1: Successful Registration]");
        registerUser("alice", "Password123!");

        // Test Case 2: Successful Login
        System.out.println("\n[Test Case 2: Successful Login]");
        loginUser("alice", "Password123!");

        // Test Case 3: Failed Login (Wrong Password)
        System.out.println("\n[Test Case 3: Failed Login - Wrong Password]");
        loginUser("alice", "WrongPassword");

        // Test Case 4: Failed Registration (Username already exists)
        System.out.println("\n[Test Case 4: Failed Registration - User Exists]");
        registerUser("alice", "AnotherPassword");

        // Test Case 5: Failed Login (User does not exist)
        System.out.println("\n[Test Case 5: Failed Login - User Not Found]");
        loginUser("bob", "Password123!");

        System.out.println("\n--- Test Cases Finished ---");
    }
}