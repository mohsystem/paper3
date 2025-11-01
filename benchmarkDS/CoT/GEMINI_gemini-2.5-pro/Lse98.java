import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

// The main class name is Lse98 as per the instructions.
public class Lse98 {

    // In a real application, this would be a database table.
    // We use a HashMap to simulate a database for this example.
    // Key: username, Value: UserData object containing salt and hashed password.
    private static final Map<String, UserData> userDatabase = new HashMap<>();

    /**
     * A simple data class to hold user's salt and hashed password.
     */
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
     * Generates a cryptographically secure salt.
     * @return a byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password with a given salt using SHA-256.
     * @param password The password to hash.
     * @param salt The salt to use for hashing.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            // Add salt to the message digest
            md.update(salt);
            // Hash the password and return the bytes
            return md.digest(password.getBytes(StandardCharsets.UTF_8));
        } catch (NoSuchAlgorithmException e) {
            // This should never happen with "SHA-256"
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a new user.
     * @param username The username.
     * @param password The password.
     * @return true if registration is successful, false otherwise (e.g., user exists).
     */
    public static boolean registerUser(String username, String password) {
        // Basic validation
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return false;
        }
        
        // In a real system, you'd use a case-insensitive check or normalize the username.
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }

        // Generate a new salt for the user
        byte[] salt = generateSalt();
        // Hash the password with the new salt
        byte[] hashedPassword = hashPassword(password, salt);

        // Store the user data in our simulated database
        // In a real app, this would be an INSERT statement with prepared statements to prevent SQL injection.
        userDatabase.put(username, new UserData(salt, hashedPassword));

        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    /**
     * Verifies a user's password.
     * @param username The username to verify.
     * @param password The password to check.
     * @return true if the password is correct, false otherwise.
     */
    public static boolean verifyPassword(String username, String password) {
        UserData userData = userDatabase.get(username);
        if (userData == null) {
            // User not found
            return false;
        }

        // Hash the provided password with the user's stored salt
        byte[] providedPasswordHash = hashPassword(password, userData.getSalt());

        // Compare the newly generated hash with the stored hash
        return Arrays.equals(providedPasswordHash, userData.getHashedPassword());
    }
    
    // Helper to convert byte array to hex string for printing
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Registration Test Cases ---");

        // Test Case 1: Successful registration
        registerUser("alice", "password123");

        // Test Case 2: Successful registration
        registerUser("bob", "secure_Pa$$w0rd");

        // Test Case 3: Attempt to register a duplicate username
        registerUser("alice", "another_password");

        // Test Case 4: Attempt to register with an empty password
        registerUser("charlie", "");

        // Test Case 5: Another successful registration
        registerUser("david", "d@v1d_r0ck$");

        System.out.println("\n--- Verifying Users ---");
        System.out.println("Login for 'alice' with correct password: " + (verifyPassword("alice", "password123") ? "Success" : "Failed"));
        System.out.println("Login for 'alice' with incorrect password: " + (verifyPassword("alice", "wrongpassword") ? "Success" : "Failed"));
        System.out.println("Login for 'bob' with correct password: " + (verifyPassword("bob", "secure_Pa$$w0rd") ? "Success" : "Failed"));
        System.out.println("Login for non-existent user 'eve': " + (verifyPassword("eve", "any_password") ? "Success" : "Failed"));

        // Print stored data for demonstration
        System.out.println("\n--- Simulated Database Content ---");
        userDatabase.forEach((username, data) -> {
            System.out.println("Username: " + username);
            System.out.println("  Salt: " + bytesToHex(data.getSalt()));
            System.out.println("  Hashed Password: " + bytesToHex(data.getHashedPassword()));
        });
    }
}