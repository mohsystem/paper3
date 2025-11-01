import java.util.HashMap;
import java.util.Map;

public class Lse34 {

    // Simulating a user database
    private static final Map<String, String> userDatabase = new HashMap<>();

    static {
        // Populate the database
        userDatabase.put("admin", "password123");
        userDatabase.put("user1", "userpass");
        userDatabase.put("john.doe", "johndoe@123");
    }

    /**
     * Checks user credentials against the database.
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return A string indicating the redirection path.
     */
    public static String login(String username, String password) {
        // Check if the username exists and the password matches
        if (userDatabase.containsKey(username) && userDatabase.get(username).equals(password)) {
            return "Login successful. Redirecting to dashboard page.";
        } else {
            return "Invalid credentials. Redirecting back to login page.";
        }
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("--- 5 Test Cases ---");

        // Test Case 1: Correct username and password
        System.out.println("Test 1 (admin, password123): " + login("admin", "password123"));

        // Test Case 2: Correct username, incorrect password
        System.out.println("Test 2 (admin, wrongpassword): " + login("admin", "wrongpassword"));

        // Test Case 3: Incorrect username
        System.out.println("Test 3 (unknownuser, password123): " + login("unknownuser", "password123"));
        
        // Test Case 4: Another valid user
        System.out.println("Test 4 (user1, userpass): " + login("user1", "userpass"));

        // Test Case 5: Case sensitivity check (username is case-sensitive)
        System.out.println("Test 5 (Admin, password123): " + login("Admin", "password123"));
    }
}