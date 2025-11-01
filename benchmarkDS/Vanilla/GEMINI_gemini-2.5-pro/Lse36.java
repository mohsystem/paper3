import java.util.HashMap;
import java.util.Map;

public class Lse36 {

    // Simulating a user database
    private static final Map<String, String> userDatabase = new HashMap<>();

    static {
        userDatabase.put("admin", "admin123");
        userDatabase.put("user_test", "password456");
        userDatabase.put("john_doe", "jd_pass");
    }

    /**
     * Simulates a login function.
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return A string indicating the login status.
     */
    public static String login(String username, String password) {
        // Check if the username exists in the database
        if (userDatabase.containsKey(username)) {
            // Check if the password is correct
            if (userDatabase.get(username).equals(password)) {
                return "Login Successful. Redirecting to home page.";
            } else {
                return "Incorrect Password.";
            }
        } else {
            return "User Not Found.";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Successful login
        System.out.println("Test 1 (admin, admin123):");
        String result1 = login("admin", "admin123");
        System.out.println("Result: " + result1 + "\n");

        // Test Case 2: Incorrect password
        System.out.println("Test 2 (user_test, wrongpassword):");
        String result2 = login("user_test", "wrongpassword");
        System.out.println("Result: " + result2 + "\n");

        // Test Case 3: User does not exist
        System.out.println("Test 3 (unknown_user, password123):");
        String result3 = login("unknown_user", "password123");
        System.out.println("Result: " + result3 + "\n");

        // Test Case 4: Another successful login
        System.out.println("Test 4 (john_doe, jd_pass):");
        String result4 = login("john_doe", "jd_pass");
        System.out.println("Result: " + result4 + "\n");
        
        // Test Case 5: Existing user, incorrect password
        System.out.println("Test 5 (admin, password456):");
        String result5 = login("admin", "password456");
        System.out.println("Result: " + result5 + "\n");
    }
}