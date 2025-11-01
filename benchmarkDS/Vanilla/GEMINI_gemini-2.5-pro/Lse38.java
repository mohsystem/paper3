import java.util.HashMap;
import java.util.Map;

public class Lse38 {

    // Simulates a user database
    private static final Map<String, String> userDatabase = new HashMap<>();

    // Static initializer to populate the database
    static {
        userDatabase.put("admin", "password123");
        userDatabase.put("johndoe", "secret");
        userDatabase.put("testuser", "test");
    }

    /**
     * Checks user credentials against the database.
     * @param username The username to check.
     * @param password The password to check.
     * @return A string indicating the result of the login attempt.
     */
    public static String login(String username, String password) {
        if (userDatabase.containsKey(username)) {
            if (userDatabase.get(username).equals(password)) {
                return "Login Successful. Redirecting to the dashboard.";
            }
        }
        return "Invalid username or password. Redirecting to the login page.";
    }

    public static void main(String[] args) {
        System.out.println("--- Java Login System Test Cases ---");

        // Test Case 1: Correct credentials
        System.out.println("Test 1 (admin, password123): " + login("admin", "password123"));

        // Test Case 2: Correct username, incorrect password
        System.out.println("Test 2 (admin, wrongpass): " + login("admin", "wrongpass"));

        // Test Case 3: Incorrect username
        System.out.println("Test 3 (user, password123): " + login("user", "password123"));

        // Test Case 4: Another set of correct credentials
        System.out.println("Test 4 (johndoe, secret): " + login("johndoe", "secret"));

        // Test Case 5: Non-existent user
        System.out.println("Test 5 (guest, guest): " + login("guest", "guest"));
    }
}