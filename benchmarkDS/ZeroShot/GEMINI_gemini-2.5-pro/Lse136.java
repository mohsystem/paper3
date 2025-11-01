import java.security.MessageDigest;
import java.nio.charset.StandardCharsets;

public class Lse136 {
    // WARNING: Storing credentials in plaintext is highly insecure.
    // In a real application, use a secure password hashing mechanism (e.g., Argon2, bcrypt)
    // and store the hashes in a secure database. This is for demonstration only.
    private static final String CORRECT_USERNAME = "admin";
    private static final String CORRECT_PASSWORD = "supersecretpassword123!";

    /**
     * Checks if the provided username and password are correct.
     * This function is designed to be called when a user visits a specific URL.
     * It uses a constant-time comparison for passwords to mitigate timing attacks.
     *
     * @param username The username obtained from the URL.
     * @param password The password obtained from the URL.
     * @return The string "true" if the credentials are correct, "false" otherwise.
     */
    public static String check_mod(String username, String password) {
        if (username == null || password == null) {
            return "false";
        }

        // Standard string comparison for username is generally acceptable,
        // as usernames are less sensitive than passwords.
        boolean isUsernameMatch = CORRECT_USERNAME.equals(username);

        // For the password, a constant-time comparison is crucial to prevent timing attacks.
        // MessageDigest.isEqual performs this type of secure comparison.
        byte[] correctPasswordBytes = CORRECT_PASSWORD.getBytes(StandardCharsets.UTF_8);
        byte[] providedPasswordBytes = password.getBytes(StandardCharsets.UTF_8);
        boolean isPasswordMatch = MessageDigest.isEqual(correctPasswordBytes, providedPasswordBytes);

        // The final result is true only if both the username and password match.
        if (isUsernameMatch && isPasswordMatch) {
            return "true";
        } else {
            return "false";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Correct credentials
        System.out.println("Test 1 (Correct): " + check_mod("admin", "supersecretpassword123!"));

        // Test Case 2: Correct username, incorrect password
        System.out.println("Test 2 (Wrong Pass): " + check_mod("admin", "wrongpassword"));

        // Test Case 3: Incorrect username, correct password
        System.out.println("Test 3 (Wrong User): " + check_mod("user", "supersecretpassword123!"));

        // Test Case 4: Incorrect credentials
        System.out.println("Test 4 (All Wrong): " + check_mod("user", "wrongpassword"));

        // Test Case 5: Null inputs
        System.out.println("Test 5 (Null Input): " + check_mod(null, "somepassword"));
    }
}