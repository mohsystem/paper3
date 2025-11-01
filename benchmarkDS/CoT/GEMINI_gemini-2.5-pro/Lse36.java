import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class Lse36 {

    // Simulates a database table of users. In a real application,
    // passwords would be hashed using a strong algorithm like BCrypt or Argon2.
    private static final Map<String, String> userDatabase = new HashMap<>();

    static {
        // Storing "hashed" passwords. Real hashes are much longer and not reversible.
        // This is a simple simulation for demonstration purposes.
        userDatabase.put("admin", hashPassword("password123"));
        userDatabase.put("jdoe", hashPassword("securePass!@#"));
        userDatabase.put("testuser", hashPassword("test"));
    }

    /**
     * Simulates hashing a password.
     * In a real system, use a standard library like BCrypt.
     * @param password The plain-text password.
     * @return A simulated hash of the password.
     */
    private static String hashPassword(String password) {
        // This is NOT a secure hash. For demonstration only.
        return new StringBuilder(password).reverse().toString() + "_hashed";
    }

    /**
     * Simulates a user login process.
     *
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return A string indicating the result of the login attempt.
     */
    public static String login(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return "Username or password cannot be empty.";
        }

        String storedHashedPassword = userDatabase.get(username);
        
        // To prevent timing attacks that could reveal valid usernames,
        // we perform a hash comparison even if the user does not exist.
        String inputHashedPassword = hashPassword(password);

        // Securely compare the provided password's hash with the stored hash.
        // `Objects.equals` provides a null-safe comparison.
        // In a real-world scenario with byte arrays for hashes, MessageDigest.isEqual()
        // should be used for a constant-time comparison.
        if (storedHashedPassword != null && storedHashedPassword.equals(inputHashedPassword)) {
            return "Login successful! Redirecting to home page...";
        } else {
            // Generic error message to prevent username enumeration.
            return "Invalid username or password.";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Login Test Cases ---");

        // Test Case 1: Successful login
        System.out.println("1. admin / password123 -> " + login("admin", "password123"));

        // Test Case 2: Incorrect password
        System.out.println("2. admin / wrongpassword -> " + login("admin", "wrongpassword"));

        // Test Case 3: Non-existent user
        System.out.println("3. unknownuser / password123 -> " + login("unknownuser", "password123"));

        // Test Case 4: Another successful login
        System.out.println("4. jdoe / securePass!@# -> " + login("jdoe", "securePass!@#"));

        // Test Case 5: Empty credentials
        System.out.println("5. \"\" / \"\" -> " + login("", ""));
    }
}