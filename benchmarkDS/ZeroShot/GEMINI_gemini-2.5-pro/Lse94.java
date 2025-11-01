import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

// The main class name should be Lse94
public class Lse94 {

    // A simple in-memory representation of a user record.
    // In a real application, this would be a row in a database table.
    private static class User {
        String username;
        byte[] salt;
        byte[] hashedPassword;

        public User(String username, byte[] salt, byte[] hashedPassword) {
            this.username = username;
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }

        @Override
        public String toString() {
            return "User{" +
                    "username='" + username + '\'' +
                    ", salt=" + bytesToHex(salt) +
                    ", hashedPassword=" + bytesToHex(hashedPassword) +
                    '}';
        }
    }
    
    // In a real application, this would be a database.
    // Using a Map to simulate a database for this example.
    private static final Map<String, User> userDatabase = new HashMap<>();
    private static final int SALT_LENGTH = 16; // 16 bytes = 128 bits

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return a byte array containing the salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password with the given salt using SHA-256.
     * NOTE: For password hashing, algorithms like Argon2, scrypt, or bcrypt are
     * recommended over SHA-256 as they are more resistant to brute-force attacks.
     * SHA-256 is used here for demonstration purposes as it's in the standard library.
     *
     * @param password The password string to hash. For better security, char[] should be used.
     * @param salt The salt to use for hashing.
     * @return a byte array containing the hashed password, or null on error.
     */
    public static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return hashedPassword;
        } catch (NoSuchAlgorithmException e) {
            // This should not happen for SHA-256
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Registers a new user by generating a salt, hashing the password,
     * and storing the information in the simulated database.
     *
     * @param username The username.
     * @param password The plain-text password.
     * @param database The map simulating the database.
     * @return true if registration is successful, false otherwise.
     */
    public static boolean registerUser(String username, String password, Map<String, User> database) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            System.err.println("Username and password cannot be empty.");
            return false;
        }
        if (database.containsKey(username)) {
            System.err.println("Username '" + username + "' already exists.");
            return false;
        }

        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);

        if (hashedPassword != null) {
            User newUser = new User(username, salt, hashedPassword);
            database.put(username, newUser);
            System.out.println("User '" + username + "' registered successfully.");
            return true;
        }
        
        System.err.println("Failed to register user '" + username + "'.");
        return false;
    }
    
    /**
     * Helper utility to convert byte array to a hexadecimal string.
     */
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }


    public static void main(String[] args) {
        // --- 5 Test Cases ---
        System.out.println("--- Running Java Test Cases ---");
        
        // Test Case 1: Standard registration
        registerUser("alice", "Password123!", userDatabase);

        // Test Case 2: Another standard registration
        registerUser("bob", "MySecureP@ssw0rd", userDatabase);

        // Test Case 3: Attempt to register a user that already exists
        registerUser("alice", "AnotherPassword", userDatabase);

        // Test Case 4: Registration with a simple password
        registerUser("charlie", "password", userDatabase);

        // Test Case 5: Registration with special characters in username (if allowed)
        registerUser("dave-the-admin", "P@$$w0rd_W1th_Ch@r$", userDatabase);

        // Print the final state of the simulated database
        System.out.println("\n--- Final Database State ---");
        userDatabase.forEach((username, user) -> System.out.println(user));
    }
}