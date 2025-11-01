// Chain-of-Through Step 1-5 are applied via secure design, constant-time compares, and hashing.
// Language: Java
// Main class must be Lse39. Includes main with 5 test cases.
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

public class Lse39 {
    // Configuration and security constants
    private static final String EXPECTED_USERNAME = "admin";
    private static final String EXPECTED_PASSWORD = "Password!234";
    private static final String USER_SALT = "usr_salt_v1|";
    private static final String PASS_SALT = "pwd_salt_v1|";
    private static final int MAX_LEN = 64;

    // Hashing utility (SHA-256)
    private static byte[] sha256(byte[] input) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            return md.digest(input);
        } catch (NoSuchAlgorithmException e) {
            // Should never happen in standard JVMs
            throw new RuntimeException("SHA-256 not available", e);
        }
    }

    private static byte[] sha256OfString(String s) {
        return sha256(s.getBytes(StandardCharsets.UTF_8));
    }

    // Constant-time comparison (arrays are always 32 bytes here)
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        // MessageDigest.isEqual uses constant time for equal length arrays
        return MessageDigest.isEqual(a, b);
    }

    // Core login function: returns result message
    public static String login(String username, String password) {
        // Basic validation and input size limits (avoid DoS, nulls)
        if (username == null || password == null) {
            return "Incorrect username or password.";
        }
        if (username.length() == 0 || password.length() == 0) {
            return "Incorrect username or password.";
        }
        if (username.length() > MAX_LEN || password.length() > MAX_LEN) {
            return "Incorrect username or password.";
        }

        // Hash the provided and expected values with separate salts to avoid length leaks
        byte[] providedUserHash = sha256OfString(USER_SALT + username);
        byte[] expectedUserHash = sha256OfString(USER_SALT + EXPECTED_USERNAME);

        byte[] providedPassHash = sha256OfString(PASS_SALT + password);
        byte[] expectedPassHash = sha256OfString(PASS_SALT + EXPECTED_PASSWORD);

        boolean userOk = constantTimeEquals(providedUserHash, expectedUserHash);
        boolean passOk = constantTimeEquals(providedPassHash, expectedPassHash);

        if (userOk && passOk) {
            return "Login successful. Redirecting to /session";
        } else {
            return "Incorrect username or password.";
        }
    }

    // Test harness with 5 test cases
    public static void main(String[] args) {
        String[] users = {
            "admin",
            "admin",
            "Admin",
            "user",
            "admin"
        };
        String[] passes = {
            "Password!234",      // correct
            "Password!2345",     // wrong password
            "Password!234",      // wrong username (case)
            "123",               // both wrong
            new String(new char[65]).replace('\0', 'a') // too long
        };

        for (int i = 0; i < users.length; i++) {
            String u = users[i];
            String p = passes[i];
            String result = login(u, p);
            System.out.println("Test " + (i + 1) + ": " + u + " / <hidden> -> " + result);
        }
    }
}