import java.util.HashMap;
import java.util.Map;

public class Lse35 {

    // This map simulates a database table of users
    private static final Map<String, String> userDatabase = new HashMap<>();

    static {
        // Populate the mock database
        userDatabase.put("admin", "admin123");
        userDatabase.put("john.doe", "password123");
        userDatabase.put("jane.doe", "securepass");
    }

    /**
     * Simulates a login function that checks credentials against a database.
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return A string indicating the result of the login attempt.
     */
    public static String login(String username, String password) {
        // Check if the username exists and the password matches
        if (userDatabase.containsKey(username) && userDatabase.get(username).equals(password)) {
            return "Login successful. Redirecting to the home page.";
        } else {
            return "Invalid username or password. Redirecting to the login page.";
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("Test Case 1: Valid credentials for 'admin'");
        System.out.println("Result: " + login("admin", "admin123"));
        System.out.println("--------------------");

        System.out.println("Test Case 2: Valid credentials for 'john.doe'");
        System.out.println("Result: " + login("john.doe", "password123"));
        System.out.println("--------------------");

        System.out.println("Test Case 3: Invalid password for 'admin'");
        System.out.println("Result: " + login("admin", "wrongpassword"));
        System.out.println("--------------------");

        System.out.println("Test Case 4: Non-existent username");
        System.out.println("Result: " + login("unknown_user", "somepassword"));
        System.out.println("--------------------");
        
        System.out.println("Test Case 5: Empty credentials");
        System.out.println("Result: " + login("", ""));
        System.out.println("--------------------");
    }
}