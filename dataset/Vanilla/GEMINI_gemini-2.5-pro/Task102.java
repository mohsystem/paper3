import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.nio.charset.StandardCharsets;

public class Task102 {

    // Simulates a database table of users. Maps username -> hashedPassword
    private static Map<String, String> database = new HashMap<>();

    /**
     * Hashes a password using SHA-256.
     * In a real application, you should use a stronger, salted hashing algorithm like bcrypt or Argon2.
     *
     * @param password The plain text password.
     * @return The SHA-256 hashed password as a hex string.
     */
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] encodedhash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            
            StringBuilder hexString = new StringBuilder(2 * encodedhash.length);
            for (int i = 0; i < encodedhash.length; i++) {
                String hex = Integer.toHexString(0xff & encodedhash[i]);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            // This should not happen for SHA-256
            throw new RuntimeException(e);
        }
    }

    /**
     * Updates the user's password in the simulated database.
     *
     * @param username The username of the user to update.
     * @param newPassword The new plain text password.
     * @param db The database map to update.
     */
    public static void updateUserPassword(String username, String newPassword, Map<String, String> db) {
        String hashedPassword = hashPassword(newPassword);
        db.put(username, hashedPassword);
        System.out.println("Password for user '" + username + "' has been updated.");
    }

    public static void main(String[] args) {
        // Initialize database with some users
        database.put("alice", hashPassword("initialPass1"));
        database.put("bob", hashPassword("initialPass2"));

        System.out.println("--- Initial Database State ---");
        database.forEach((user, pass) -> System.out.println("User: " + user + ", Hashed Pass: " + pass));
        System.out.println("\n--- Running 5 Test Cases ---");

        // Test Case 1: Update existing user 'alice'
        updateUserPassword("alice", "newSecurePassword123", database);

        // Test Case 2: Update existing user 'bob'
        updateUserPassword("bob", "anotherPassword!@#", database);

        // Test Case 3: Add a new user 'charlie'
        updateUserPassword("charlie", "charliePass2024", database);
        
        // Test Case 4: Update user 'alice' again
        updateUserPassword("alice", "evenNewerPassword$", database);

        // Test Case 5: Add a new user 'david'
        updateUserPassword("david", "d@v1d$P@$$", database);
        
        System.out.println("\n--- Final Database State ---");
        database.forEach((user, pass) -> System.out.println("User: " + user + ", Hashed Pass: " + pass));
    }
}