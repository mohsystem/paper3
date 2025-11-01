import java.util.Map;
import java.util.HashMap;
import java.security.MessageDigest;
import java.util.Arrays;

// The main class name must be Lse37 for the code to run
public class Lse37 {

    // In a real application, this would be a secure database connection.
    // Passwords must be stored as salted hashes using a strong algorithm like Argon2, scrypt, or bcrypt.
    // This map is a simplified simulation for demonstration purposes.
    private static final Map<String, String> userDatabase = new HashMap<>();

    static {
        // Storing "hashed" passwords. Real hashes are much longer.
        // Example: "password123" -> some_hash_value
        userDatabase.put("admin", "hashed_password_for_admin");
        // Example: "testuser" -> another_password
        userDatabase.put("testuser", "hashed_password_for_user");
    }

    /**
     * Performs a constant-time comparison of two byte arrays to prevent timing attacks.
     *
     * @param a The first byte array.
     * @param b The second byte array.
     * @return true if the arrays are equal, false otherwise.
     */
    private static boolean secureEquals(String a, String b) {
        // Using MessageDigest.isEqual is a standard, secure way to do this in Java.
        return MessageDigest.isEqual(a.getBytes(), b.getBytes());
    }

    /**
     * Simulates a user login process.
     *
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return A string indicating the result: "success", "cant_login", or "internal_server_error".
     */
    public static String login(String username, String password) {
        try {
            // 1. Input Validation: Ensure username and password are not null or empty.
            if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
                return "cant_login";
            }

            // 2. Database Lookup: Retrieve the stored "hash" for the user.
            String storedHashedPassword = userDatabase.get(username);

            // 3. User Existence Check: If no user is found, return a generic error.
            // This prevents user enumeration attacks.
            if (storedHashedPassword == null) {
                // To make the timing consistent, we can perform a dummy comparison.
                secureEquals("dummy_hash_value_to_avoid_timing_leak", password);
                return "cant_login";
            }

            // 4. Password Verification: In a real app, you would hash the input password
            // with the user's salt and then compare the result with the stored hash.
            // Here, we simulate this by comparing the plaintext password with the stored "hash".
            // The comparison MUST be done in constant time to prevent timing attacks.
            if (secureEquals(storedHashedPassword, password)) {
                return "success";
            } else {
                return "cant_login";
            }

        } catch (Exception e) {
            // 5. Error Handling: Catch any unexpected exceptions.
            System.err.println("An unexpected error occurred: " + e.getMessage());
            return "internal_server_error";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Successful login
        String result1 = login("admin", "hashed_password_for_admin");
        System.out.println("Test Case 1 (Correct Credentials): " + result1);

        // Test Case 2: Incorrect password
        String result2 = login("admin", "wrong_password");
        System.out.println("Test Case 2 (Incorrect Password): " + result2);

        // Test Case 3: Non-existent user
        String result3 = login("unknown_user", "some_password");
        System.out.println("Test Case 3 (Non-existent User): " + result3);

        // Test Case 4: Null input
        String result4 = login(null, "some_password");
        System.out.println("Test Case 4 (Null Username): " + result4);

        // Test Case 5: Empty input
        String result5 = login("testuser", "");
        System.out.println("Test Case 5 (Empty Password): " + result5);

        System.out.println("--- Test Cases Finished ---");
    }
}