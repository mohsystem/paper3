import java.security.MessageDigest;
import java.nio.charset.StandardCharsets;

public class Lse135 {

    // WARNING: This is a simplified example for demonstration purposes only.
    // DO NOT use hardcoded credentials or plaintext password comparison in production applications.
    // In a real-world scenario, passwords should be securely stored as salted hashes
    // using a strong algorithm like Argon2, scrypt, or PBKDF2, and credentials should
    // be managed through a secure configuration or secrets management system.
    private static final String CORRECT_USERNAME = "admin";
    private static final byte[] CORRECT_PASSWORD_BYTES = "Pa$$w0rd123!".getBytes(StandardCharsets.UTF_8);

    /**
     * Checks if the provided username and password are correct.
     * This function simulates checking credentials from a form.
     *
     * @param username The username to check.
     * @param password The password to check.
     * @return true if the credentials are correct, false otherwise.
     */
    public static boolean check_mod(String username, String password) {
        // Rule#1: Ensure all input is validated. Check for null inputs.
        if (username == null || password == null) {
            return false;
        }

        // Username can be compared directly, as it's not typically a secret.
        boolean usernameMatch = CORRECT_USERNAME.equals(username);

        // Use a constant-time comparison for the password to mitigate timing attacks.
        // This is crucial for comparing secrets.
        byte[] providedPasswordBytes = password.getBytes(StandardCharsets.UTF_8);
        boolean passwordMatch = MessageDigest.isEqual(CORRECT_PASSWORD_BYTES, providedPasswordBytes);

        return usernameMatch && passwordMatch;
    }

    /**
     * Main method to run test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("Test Case 1: Correct credentials");
        System.out.println("Input: user='admin', pass='Pa$$w0rd123!'");
        System.out.println("Result: " + check_mod("admin", "Pa$$w0rd123!")); // Expected: true

        System.out.println("\nTest Case 2: Incorrect username");
        System.out.println("Input: user='user', pass='Pa$$w0rd123!'");
        System.out.println("Result: " + check_mod("user", "Pa$$w0rd123!")); // Expected: false

        System.out.println("\nTest Case 3: Incorrect password");
        System.out.println("Input: user='admin', pass='wrongpassword'");
        System.out.println("Result: " + check_mod("admin", "wrongpassword")); // Expected: false

        System.out.println("\nTest Case 4: Null input for username");
        System.out.println("Input: user=null, pass='Pa$$w0rd123!'");
        System.out.println("Result: " + check_mod(null, "Pa$$w0rd123!")); // Expected: false

        System.out.println("\nTest Case 5: Empty inputs");
        System.out.println("Input: user='', pass=''");
        System.out.println("Result: " + check_mod("", "")); // Expected: false
    }
}