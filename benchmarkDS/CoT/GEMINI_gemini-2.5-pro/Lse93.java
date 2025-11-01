import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse93 {

    /**
     * Hashes a password using SHA-256 and simulates inserting the user record.
     *
     * @param username The username to be stored.
     * @param password The plaintext password to be hashed and stored.
     * @return The hex-encoded SHA-256 hash of the password, or null on error.
     */
    public static String registerUser(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            System.out.println("Error: Username and password cannot be empty.");
            return null;
        }

        try {
            // Create a SHA-256 MessageDigest instance
            MessageDigest digest = MessageDigest.getInstance("SHA-256");

            // Get the hash of the password bytes
            byte[] encodedhash = digest.digest(
                    password.getBytes(StandardCharsets.UTF_8));

            // Convert byte array to a hex string
            StringBuilder hexString = new StringBuilder(2 * encodedhash.length);
            for (byte b : encodedhash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            String hashedPassword = hexString.toString();

            // Simulate inserting into the database
            System.out.println("--- User Registration ---");
            System.out.println("Simulating database insertion...");
            System.out.println("Username: " + username);
            System.out.println("Stored Hashed Password: " + hashedPassword);
            System.out.println("-------------------------");

            return hashedPassword;

        } catch (NoSuchAlgorithmException e) {
            System.err.println("Error: SHA-256 algorithm not found.");
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases...");

        // Test Case 1
        registerUser("alice", "Password123!");

        // Test Case 2
        registerUser("bob", "my$ecr3tP@ssw0rd");

        // Test Case 3
        registerUser("charlie", "admin");

        // Test Case 4
        registerUser("dave", "daveslongandsecurepassword");

        // Test Case 5: Empty password test
        registerUser("eve", "");
    }
}