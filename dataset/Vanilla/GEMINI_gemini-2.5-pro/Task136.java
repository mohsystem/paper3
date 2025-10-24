import java.util.HashMap;
import java.util.Map;

public class Task136 {

    // In a real application, passwords should be hashed and stored securely.
    // For this example, we use a simple map with plain text for demonstration.
    private static final Map<String, String> userDatabase = new HashMap<>();

    static {
        userDatabase.put("admin", "password123");
        userDatabase.put("user1", "securepass");
        userDatabase.put("john.doe", "johndoe_pass");
    }

    /**
     * Authenticates a user based on username and password.
     * @param username The username to check.
     * @param password The password to verify.
     * @return true if authentication is successful, false otherwise.
     */
    public static boolean authenticate(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        String storedPassword = userDatabase.get(username);
        if (storedPassword != null && storedPassword.equals(password)) {
            return true;
        }
        
        return false;
    }

    public static void main(String[] args) {
        System.out.println("--- User Authentication Test Cases ---");

        // Test Case 1: Successful login
        boolean result1 = authenticate("admin", "password123");
        System.out.println("1. admin / password123 -> " + (result1 ? "Success" : "Failure"));

        // Test Case 2: Wrong password
        boolean result2 = authenticate("user1", "wrongpass");
        System.out.println("2. user1 / wrongpass -> " + (result2 ? "Success" : "Failure"));

        // Test Case 3: Non-existent user
        boolean result3 = authenticate("unknown_user", "password123");
        System.out.println("3. unknown_user / password123 -> " + (result3 ? "Success" : "Failure"));

        // Test Case 4: Case-sensitive username
        boolean result4 = authenticate("Admin", "password123");
        System.out.println("4. Admin / password123 -> " + (result4 ? "Success" : "Failure"));

        // Test Case 5: Null input
        boolean result5 = authenticate(null, "some_password");
        System.out.println("5. null / some_password -> " + (result5 ? "Success" : "Failure"));
    }
}