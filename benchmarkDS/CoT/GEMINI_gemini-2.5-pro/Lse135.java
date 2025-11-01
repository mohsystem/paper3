import java.security.MessageDigest;
import java.util.Arrays;

public class Lse135 {

    // NOTE: Storing credentials in plaintext is a major security risk.
    // In a real application, use a secure method like hashed and salted passwords.
    private static final String CORRECT_USERNAME = "admin";
    private static final String CORRECT_PASSWORD = "password123";

    /**
     * Checks if the provided username and password are correct.
     *
     * @param username The username to check.
     * @param password The password to check.
     * @return true if credentials are correct, false otherwise.
     */
    public static boolean check_mod(String username, String password) {
        if (username == null || password == null) {
            return false;
        }

        // Standard string comparison is fine for username
        boolean isUsernameCorrect = CORRECT_USERNAME.equals(username);

        // Use a constant-time comparison for the password to prevent timing attacks.
        byte[] correctPasswordBytes = CORRECT_PASSWORD.getBytes();
        byte[] providedPasswordBytes = password.getBytes();
        
        boolean isPasswordCorrect = MessageDigest.isEqual(correctPasswordBytes, providedPasswordBytes);

        return isUsernameCorrect && isPasswordCorrect;
    }

    public static void main(String[] args) {
        // Test Case 1: Correct username and password
        System.out.println("Test Case 1 (Correct credentials): " + check_mod("admin", "password123"));

        // Test Case 2: Correct username, incorrect password
        System.out.println("Test Case 2 (Incorrect password): " + check_mod("admin", "wrongpassword"));

        // Test Case 3: Incorrect username, correct password
        System.out.println("Test Case 3 (Incorrect username): " + check_mod("user", "password123"));

        // Test Case 4: Incorrect username and password
        System.out.println("Test Case 4 (Incorrect credentials): " + check_mod("user", "wrongpassword"));

        // Test Case 5: Null inputs
        System.out.println("Test Case 5 (Null input): " + check_mod("admin", null));
    }
}