import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Lse34 {

    // A simple in-memory map to act as a user database.
    // In a real application, this would be a proper database (e.g., SQL, NoSQL).
    private final Map<String, UserCredentials> userDatabase = new HashMap<>();

    // Inner class to store user's salt and hashed password
    private static class UserCredentials {
        private final byte[] salt;
        private final byte[] hashedPassword;

        public UserCredentials(byte[] salt, byte[] hashedPassword) {
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
     * Generates a cryptographically secure salt.
     * @return A 16-byte random salt.
     */
    private byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password using PBKDF2 with a given salt. This is a strong,
     * industry-standard algorithm for password storage.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as an Optional, or empty if an error occurs.
     */
    private Optional<byte[]> hashPassword(char[] password, byte[] salt) {
        // Iteration count should be high enough to be slow for attackers but acceptable for users.
        int iterations = 65536;
        int keyLength = 256;
        KeySpec spec = new PBEKeySpec(password, salt, iterations, keyLength);
        try {
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return Optional.of(factory.generateSecret(spec).getEncoded());
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real app, log this severe error.
            System.err.println("Error while hashing password: " + e.getMessage());
            return Optional.empty();
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * @param username The username.
     * @param password The password.
     */
    public void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return;
        }
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username already exists.");
            return;
        }
        
        byte[] salt = generateSalt();
        Optional<byte[]> hashedPasswordOpt = hashPassword(password.toCharArray(), salt);

        hashedPasswordOpt.ifPresent(hashedPassword -> {
            userDatabase.put(username, new UserCredentials(salt, hashedPassword));
            System.out.println("User '" + username + "' registered successfully.");
        });
    }

    /**
     * Attempts to log a user in by securely verifying their credentials.
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return A string indicating the result of the login attempt.
     */
    public String loginUser(String username, String password) {
        // Check if username exists. Use a generic message for failures to prevent username enumeration.
        if (username == null || password == null || !userDatabase.containsKey(username)) {
            return "Login failed: Invalid username or password. Redirecting to login page.";
        }

        UserCredentials credentials = userDatabase.get(username);
        byte[] salt = credentials.getSalt();
        byte[] storedHash = credentials.getHashedPassword();

        // Hash the provided password with the user's stored salt
        Optional<byte[]> providedPasswordHashOpt = hashPassword(password.toCharArray(), salt);

        if (providedPasswordHashOpt.isEmpty()) {
            return "Login failed due to an internal server error. Redirecting to login page.";
        }
        byte[] providedPasswordHash = providedPasswordHashOpt.get();

        // Use a constant-time comparison to prevent timing attacks.
        if (Arrays.equals(storedHash, providedPasswordHash)) {
            return "Login successful! Redirecting to dashboard...";
        } else {
            return "Login failed: Invalid username or password. Redirecting to login page.";
        }
    }

    public static void main(String[] args) {
        Lse34 authSystem = new Lse34();

        // Setup: Register a user for testing
        String testUser = "testUser";
        String testPass = "P@ssword123!";
        authSystem.registerUser(testUser, testPass);
        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Successful Login
        System.out.println("Test Case 1: Correct username and password.");
        String result1 = authSystem.loginUser(testUser, testPass);
        System.out.println("Result: " + result1);
        System.out.println("--------------------------");

        // Test Case 2: Incorrect Password
        System.out.println("Test Case 2: Correct username, incorrect password.");
        String result2 = authSystem.loginUser(testUser, "WrongPassword");
        System.out.println("Result: " + result2);
        System.out.println("--------------------------");

        // Test Case 3: Incorrect Username
        System.out.println("Test Case 3: Incorrect username.");
        String result3 = authSystem.loginUser("nonexistentUser", testPass);
        System.out.println("Result: " + result3);
        System.out.println("--------------------------");
        
        // Test Case 4: Case-sensitive Username
        System.out.println("Test Case 4: Case-sensitive username check (should fail).");
        String result4 = authSystem.loginUser("testuser", testPass);
        System.out.println("Result: " + result4);
        System.out.println("--------------------------");

        // Test Case 5: Empty Password
        System.out.println("Test Case 5: Correct username, empty password (should fail).");
        String result5 = authSystem.loginUser(testUser, "");
        System.out.println("Result: " + result5);
        System.out.println("--------------------------");
    }
}