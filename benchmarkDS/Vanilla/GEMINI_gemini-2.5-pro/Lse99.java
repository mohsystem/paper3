import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

public class Lse99 {

    // Simulating a database with a HashMap
    private static final Map<String, String> database = new HashMap<>();

    /**
     * Hashes a password using the SHA-256 algorithm.
     * @param password The plain text password.
     * @return A hex string representation of the hashed password.
     */
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] encodedhash = digest.digest(password.getBytes(StandardCharsets.UTF_8));

            StringBuilder hexString = new StringBuilder(2 * encodedhash.length);
            for (byte b : encodedhash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            // In a real application, this should be handled more gracefully.
            throw new RuntimeException("Hashing algorithm not found", e);
        }
    }

    /**
     * "Inserts" a new user into the simulated database.
     * @param username The username for the new user.
     * @param password The password for the new user.
     * @return true if the user was successfully registered, false otherwise.
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            System.out.println("Error: Username and password cannot be empty.");
            return false;
        }
        if (database.containsKey(username)) {
            System.out.println("Error: Username '" + username + "' already exists.");
            return false;
        }

        String hashedPassword = hashPassword(password);
        database.put(username, hashedPassword);
        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 User Registration Test Cases ---");

        // Test Case 1
        registerUser("alice", "password123");

        // Test Case 2
        registerUser("bob", "my$ecretP@ss");

        // Test Case 3
        registerUser("charlie", "Charlie'sPassword!");

        // Test Case 4
        registerUser("david", "d@v1d_R0ck$");

        // Test Case 5
        registerUser("eve", "spyware");

        System.out.println("\n--- Final Database State ---");
        for (Map.Entry<String, String> entry : database.entrySet()) {
            System.out.println("Username: " + entry.getKey() + ", Hashed Password: " + entry.getValue());
        }
    }
}