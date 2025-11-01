import java.util.HashMap;
import java.util.Map;

public class Lse57 {

    // In-memory database to store user information.
    // Key: username, Value: User object.
    // In a real application, this would be a persistent database.
    private static Map<String, User> userDatabase = new HashMap<>();
    
    // Represents the currently logged-in user. null if no one is logged in.
    private static String loggedInUser = null;

    // User class to hold user data.
    static class User {
        String username;
        String email;
        String password; // In a real app, this should be a salted password hash.

        User(String username, String email, String password) {
            this.username = username;
            this.email = email;
            this.password = password;
        }

        @Override
        public String toString() {
            return "User{" +
                    "username='" + username + '\'' +
                    ", email='" + email + '\'' +
                    '}';
        }
    }

    /**
     * Simulates a user logging in.
     * @param username The username of the user.
     * @param password The password of the user.
     * @return true if login is successful, false otherwise.
     */
    public static boolean login(String username, String password) {
        User user = userDatabase.get(username);
        if (user != null && user.password.equals(password)) {
            loggedInUser = username;
            System.out.println("Login successful for: " + username);
            return true;
        }
        System.out.println("Login failed for: " + username);
        return false;
    }

    /**
     * Simulates a user logging out.
     */
    public static void logout() {
        System.out.println("User " + loggedInUser + " logged out.");
        loggedInUser = null;
    }

    /**
     * Changes the email for the currently logged-in user.
     * @param oldEmail The user's current email address.
     * @param newEmail The desired new email address.
     * @param confirmPassword The user's password for verification.
     * @return A status message indicating success or failure.
     */
    public static String changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        // 1. Authorization: Check if a user is logged in.
        if (loggedInUser == null) {
            return "Error: You must be logged in to change your email.";
        }

        User currentUser = userDatabase.get(loggedInUser);

        // 2. Validation: Check if the provided old email matches the logged-in user's record.
        if (currentUser == null || !currentUser.email.equals(oldEmail)) {
            return "Error: The old email address provided is incorrect.";
        }

        // 3. Authentication: Verify the user's password.
        // NOTE: In a real system, use a constant-time comparison for security.
        if (!currentUser.password.equals(confirmPassword)) {
            return "Error: Incorrect password.";
        }
        
        // 4. Input Validation: Check if the new email is not null or empty
        if (newEmail == null || newEmail.trim().isEmpty() || !newEmail.contains("@")) {
            return "Error: New email is not valid.";
        }
        
        // 5. Uniqueness Check: Ensure the new email is not already in use by another user.
        for (User user : userDatabase.values()) {
            if (user.email.equalsIgnoreCase(newEmail) && !user.username.equals(loggedInUser)) {
                return "Error: The new email address is already in use by another account.";
            }
        }

        // 6. Update the email
        currentUser.email = newEmail;
        return "Success: Email has been changed to " + newEmail;
    }

    public static void main(String[] args) {
        // Setup initial user data
        userDatabase.put("john.doe", new User("john.doe", "john.d@example.com", "password123"));
        userDatabase.put("jane.doe", new User("jane.doe", "jane.d@example.com", "password456"));

        System.out.println("--- Test Case 1: Successful Email Change ---");
        login("john.doe", "password123");
        String result1 = changeEmail("john.d@example.com", "john.new@example.com", "password123");
        System.out.println("Result: " + result1);
        System.out.println("Current user info: " + userDatabase.get("john.doe"));
        logout();
        System.out.println("\n--------------------------------------------\n");

        System.out.println("--- Test Case 2: Attempt to Change Email While Logged Out ---");
        String result2 = changeEmail("jane.d@example.com", "jane.new@example.com", "password456");
        System.out.println("Result: " + result2);
        System.out.println("\n--------------------------------------------\n");

        System.out.println("--- Test Case 3: Incorrect Password ---");
        login("jane.doe", "password456");
        String result3 = changeEmail("jane.d@example.com", "jane.new@example.com", "wrongpassword");
        System.out.println("Result: " + result3);
        logout();
        System.out.println("\n--------------------------------------------\n");

        System.out.println("--- Test Case 4: Incorrect Old Email ---");
        login("john.doe", "password123");
        String result4 = changeEmail("wrong.email@example.com", "john.another@example.com", "password123");
        System.out.println("Result: " + result4);
        logout();
        System.out.println("\n--------------------------------------------\n");

        System.out.println("--- Test Case 5: New Email Already in Use ---");
        login("john.doe", "password123");
        // john tries to take jane's email
        String result5 = changeEmail("john.new@example.com", "jane.d@example.com", "password123");
        System.out.println("Result: " + result5);
        logout();
        System.out.println("\n--------------------------------------------\n");
    }
}