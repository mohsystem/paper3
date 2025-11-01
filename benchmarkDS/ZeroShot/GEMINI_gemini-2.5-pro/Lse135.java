import java.security.MessageDigest;
import java.nio.charset.StandardCharsets;

public class Lse135 {

    // In a real application, these would not be hardcoded.
    // The password would be a salted hash, not plaintext.
    private static final String CORRECT_USERNAME = "admin";
    private static final String CORRECT_PASSWORD = "P@ssw0rd!123";

    /**
     * Securely checks if the provided username and password are correct.
     * Uses a constant-time comparison to prevent timing attacks.
     *
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return true if credentials are correct, false otherwise.
     */
    public static boolean check_mod(String username, String password) {
        if (username == null || password == null) {
            return false;
        }

        // Convert strings to byte arrays for comparison
        byte[] correctUserBytes = CORRECT_USERNAME.getBytes(StandardCharsets.UTF_8);
        byte[] providedUserBytes = username.getBytes(StandardCharsets.UTF_8);
        byte[] correctPassBytes = CORRECT_PASSWORD.getBytes(StandardCharsets.UTF_8);
        byte[] providedPassBytes = password.getBytes(StandardCharsets.UTF_8);

        // Use constant-time comparison to mitigate timing attacks.
        boolean userMatch = MessageDigest.isEqual(correctUserBytes, providedUserBytes);
        boolean passMatch = MessageDigest.isEqual(correctPassBytes, providedPassBytes);

        return userMatch && passMatch;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Correct credentials");
        System.out.println("Input: admin, P@ssw0rd!123");
        System.out.println("Output: " + check_mod("admin", "P@ssw0rd!123")); // Expected: true
        System.out.println("--------------------");

        System.out.println("Test Case 2: Correct username, incorrect password");
        System.out.println("Input: admin, wrongpassword");
        System.out.println("Output: " + check_mod("admin", "wrongpassword")); // Expected: false
        System.out.println("--------------------");
        
        System.out.println("Test Case 3: Incorrect username, correct password");
        System.out.println("Input: user, P@ssw0rd!123");
        System.out.println("Output: " + check_mod("user", "P@ssw0rd!123")); // Expected: false
        System.out.println("--------------------");

        System.out.println("Test Case 4: Incorrect credentials");
        System.out.println("Input: user, password");
        System.out.println("Output: " + check_mod("user", "password")); // Expected: false
        System.out.println("--------------------");
        
        System.out.println("Test Case 5: Correct username, password with different length");
        System.out.println("Input: admin, P@ss");
        System.out.println("Output: " + check_mod("admin", "P@ss")); // Expected: false
        System.out.println("--------------------");
    }
}