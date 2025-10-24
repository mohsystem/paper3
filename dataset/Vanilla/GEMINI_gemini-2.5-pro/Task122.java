import java.util.HashMap;
import java.util.Map;

public class Task122 {

    // A simple in-memory map to store user data.
    // In a real application, this would be a database.
    private static Map<String, String> userDatabase = new HashMap<>();

    /**
     * Signs up a new user by storing their username and password.
     * NOTE: In a real-world application, passwords should NEVER be stored in plaintext.
     * They should be securely hashed using an algorithm like bcrypt or Argon2.
     *
     * @param username The username to register.
     * @param password The password for the new user.
     * @return true if registration is successful, false if the username already exists.
     */
    public static boolean signUp(String username, String password) {
        if (username == null || password == null || username.isEmpty()) {
            return false; // Basic validation
        }
        if (userDatabase.containsKey(username)) {
            System.out.println("Signup failed: Username '" + username + "' already exists.");
            return false; // Username already exists
        }
        userDatabase.put(username, password);
        System.out.println("Signup successful: User '" + username + "' created.");
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Successful signup for a new user
        signUp("alice", "password123");

        // Test Case 2: Successful signup for another new user
        signUp("bob", "securePass_456");

        // Test Case 3: Failed signup with an existing username
        signUp("alice", "anotherPassword");

        // Test Case 4: Successful signup with a user whose name is a number
        signUp("user789", "mySecret");

        // Test Case 5: Successful signup with an empty password (for demonstration)
        signUp("charlie", "");

        // Print final state of the database
        System.out.println("\nCurrent user database: " + userDatabase);
    }
}