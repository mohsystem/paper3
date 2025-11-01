import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

public class Lse39 {

    // NOTE: Storing credentials directly in the code is highly insecure.
    // In a real application, use a secure vault, environment variables,
    // and store password hashes (e.g., using Argon2) instead of plaintext.
    private static final String CORRECT_USERNAME = "admin";
    private static final byte[] CORRECT_PASSWORD_BYTES = "password123!".getBytes(StandardCharsets.UTF_8);
    private static final int MAX_INPUT_LENGTH = 128;

    /**
     * Validates user credentials in a secure manner.
     *
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return true if login is successful, false otherwise.
     */
    public static boolean login(String username, String password) {
        // Rule #1: Ensure all input is validated.
        if (username == null || username.isEmpty() || username.length() > MAX_INPUT_LENGTH) {
            System.out.println("Invalid username format.");
            return false;
        }
        if (password == null || password.isEmpty() || password.length() > MAX_INPUT_LENGTH) {
            System.out.println("Invalid password format.");
            return false;
        }

        // Standard comparison for username (not a secret).
        boolean usernameMatches = CORRECT_USERNAME.equals(username);

        byte[] passwordBytes = password.getBytes(StandardCharsets.UTF_8);

        // Rule #7 & #8: Use constant-time comparison for secrets to prevent timing attacks.
        boolean passwordMatches = MessageDigest.isEqual(passwordBytes, CORRECT_PASSWORD_BYTES);

        if (usernameMatches && passwordMatches) {
            System.out.println("Login successful! Redirecting to session page...");
            return true;
        } else {
            System.out.println("Incorrect username or password.");
            return false;
        }
    }

    /**
     * Main method with test cases.
     */
    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Correct credentials ---");
        login("admin", "password123!");

        System.out.println("\n--- Test Case 2: Incorrect password ---");
        login("admin", "wrongpassword");

        System.out.println("\n--- Test Case 3: Incorrect username ---");
        login("user", "password123!");

        System.out.println("\n--- Test Case 4: Null input ---");
        login(null, "password123!");

        System.out.println("\n--- Test Case 5: Empty input ---");
        login("admin", "");
    }
}