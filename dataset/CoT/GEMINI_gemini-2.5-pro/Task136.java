import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

public class Task136 {

    // In-memory storage for username -> hashedPassword.
    // In a real application, this would be a database.
    private final Map<String, String> userDatabase;

    public Task136() {
        userDatabase = new HashMap<>();
        // Pre-populate with some users. Passwords should be hashed before storing.
        // Example: "password123" -> "ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f"
        // Example: "adminPass!" -> "1b87a86c6d2038314e3843b0927e85489839473b13411c5240212f458133503f"
        userDatabase.put("user1", "ef92b778bafe771e89245b89ecbc08a44a4e166c06659911881f383d4473e94f");
        userDatabase.put("admin", "1b87a86c6d2038314e3843b0927e85489839473b13411c5240212f458133503f");
    }

    /**
     * Hashes a password using SHA-256.
     * @param password The plaintext password.
     * @return The hex string representation of the hashed password.
     */
    private String hashPassword(String password) {
        if (password == null) {
            return null;
        }
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
            // This should not happen for SHA-256
            throw new RuntimeException("Hashing algorithm not found", e);
        }
    }

    /**
     * Authenticates a user against the stored credentials.
     * @param username The username to authenticate.
     * @param password The password to check.
     * @return true if authentication is successful, false otherwise.
     */
    public boolean authenticateUser(String username, String password) {
        // 1. Basic input validation
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return false;
        }

        // 2. Check if user exists
        String storedHash = userDatabase.get(username);
        if (storedHash == null) {
            return false;
        }

        // 3. Hash the provided password
        String inputHash = hashPassword(password);

        // 4. Securely compare the hashes (constant-time comparison)
        return MessageDigest.isEqual(storedHash.getBytes(StandardCharsets.UTF_8), inputHash.getBytes(StandardCharsets.UTF_8));
    }

    public static void main(String[] args) {
        Task136 auth = new Task136();

        // --- Test Cases ---
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Correct username, correct password
        boolean result1 = auth.authenticateUser("user1", "password123");
        System.out.println("Test 1 (user1, password123): " + (result1 ? "Success" : "Failure"));

        // Test Case 2: Correct username, incorrect password
        boolean result2 = auth.authenticateUser("user1", "wrongpassword");
        System.out.println("Test 2 (user1, wrongpassword): " + (result2 ? "Success" : "Failure"));

        // Test Case 3: Incorrect username
        boolean result3 = auth.authenticateUser("unknownuser", "password123");
        System.out.println("Test 3 (unknownuser, password123): " + (result3 ? "Success" : "Failure"));

        // Test Case 4: Correct username, empty password
        boolean result4 = auth.authenticateUser("admin", "");
        System.out.println("Test 4 (admin, ''): " + (result4 ? "Success" : "Failure"));

        // Test Case 5: Null username
        boolean result5 = auth.authenticateUser(null, "somepass");
        System.out.println("Test 5 (null, somepass): " + (result5 ? "Success" : "Failure"));
    }
}