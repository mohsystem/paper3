import java.util.HashMap;
import java.util.Map;

public class Lse37 {
    // In-memory map to simulate a user database
    private static final Map<String, String> database = new HashMap<>();

    static {
        // Populate the database with some users
        database.put("admin", "password123");
        database.put("user", "userpass");
        database.put("testuser", "test");
    }

    /**
     * Checks user credentials against the database.
     * @param username The username to check.
     * @param password The password to check.
     * @return "success" on match, "cant_login" on mismatch, "internal_server_error" on unexpected issues.
     */
    public static String login(String username, String password) {
        try {
            if (username == null || password == null) {
                return "cant_login"; // Treat null input as a failed login attempt
            }

            // Check if the username exists and the password matches
            if (database.containsKey(username) && database.get(username).equals(password)) {
                return "success";
            } else {
                return "cant_login";
            }
        } catch (Exception e) {
            // Catch any unexpected exceptions
            return "internal_server_error";
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Correct username and password
        System.out.println("Test 1: login(\"admin\", \"password123\") -> " + login("admin", "password123"));

        // Test Case 2: Correct username, incorrect password
        System.out.println("Test 2: login(\"user\", \"wrongpassword\") -> " + login("user", "wrongpassword"));

        // Test Case 3: Incorrect username
        System.out.println("Test 3: login(\"unknown\", \"userpass\") -> " + login("unknown", "userpass"));

        // Test Case 4: Empty credentials
        System.out.println("Test 4: login(\"\", \"\") -> " + login("", ""));

        // Test Case 5: Another correct username and password
        System.out.println("Test 5: login(\"testuser\", \"test\") -> " + login("testuser", "test"));
    }
}