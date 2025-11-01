import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.Base64;

class Lse56 {

    // --- Mock User Database and Session ---
    // In a real application, this would be a database.
    private static final Map<String, User> userDatabase = new HashMap<>();
    // In a real application, session management would be more robust (e.g., JWT, session IDs).
    private static User currentUser = null;

    // User class to hold user data
    private static class User {
        String username;
        String email;
        String passwordHash;

        User(String username, String email, String password) {
            this.username = username;
            this.email = email;
            // In a real application, use a strong hashing algorithm like BCrypt or Argon2.
            // This is a simple mock for demonstration.
            this.passwordHash = hashPassword(password);
        }
    }

    // --- Security Utilities (Mocks) ---
    private static String hashPassword(String password) {
        // MOCK HASHING: NOT FOR PRODUCTION USE.
        // Use libraries like BCrypt, SCrypt, or Argon2 in a real application.
        return Base64.getEncoder().encodeToString(("salted_" + password).getBytes());
    }

    private static boolean verifyPassword(String plainPassword, String storedHash) {
        // MOCK VERIFICATION: NOT FOR PRODUCTION USE.
        // This comparison should be done in a constant-time manner to prevent timing attacks.
        if (storedHash == null || plainPassword == null) {
            return false;
        }
        String newHash = hashPassword(plainPassword);
        return newHash.equals(storedHash);
    }
    
    // Basic email format validation
    private static boolean isValidEmailFormat(String email) {
        if (email == null) return false;
        // A simple check, a more robust regex would be used in production.
        return email.contains("@") && email.contains(".");
    }

    // --- Core Functionality ---
    public static boolean login(String username, String password) {
        User user = userDatabase.get(username);
        if (user != null && verifyPassword(password, user.passwordHash)) {
            currentUser = user;
            return true;
        }
        currentUser = null;
        return false;
    }

    public static void logout() {
        currentUser = null;
    }

    /**
     * Changes the user's email address after performing security checks.
     * @param oldEmail The user's current email address for verification.
     * @param newEmail The desired new email address.
     * @param password The user's current password for authorization.
     * @return A string indicating the result of the operation.
     */
    public static String changeEmail(String oldEmail, String newEmail, String password) {
        // 1. Check if a user is logged in
        if (currentUser == null) {
            return "Error: You must be logged in to change your email.";
        }

        // 2. Verify the provided password is correct for the logged-in user
        if (!verifyPassword(password, currentUser.passwordHash)) {
            return "Error: Incorrect password.";
        }

        // 3. Verify the provided old email matches the one on record
        if (!Objects.equals(currentUser.email, oldEmail)) {
            return "Error: The old email address does not match our records.";
        }

        // 4. Check if the new email is the same as the old email
        if (Objects.equals(oldEmail, newEmail)) {
            return "Error: New email cannot be the same as the old email.";
        }
        
        // 5. Validate the new email format
        if (!isValidEmailFormat(newEmail)) {
            return "Error: Invalid new email format.";
        }

        // All checks passed, update the email
        currentUser.email = newEmail;
        return "Success: Email has been updated to " + newEmail;
    }

    // --- Main method with test cases ---
    public static void main(String[] args) {
        // Setup: Create a user and add to the mock database
        User testUser = new User("testuser", "user@example.com", "Password123!");
        userDatabase.put(testUser.username, testUser);

        System.out.println("--- Test Case 1: Successful Email Change ---");
        login("testuser", "Password123!");
        System.out.println("Login successful for: " + (currentUser != null ? currentUser.username : "none"));
        String result1 = changeEmail("user@example.com", "new.user@example.com", "Password123!");
        System.out.println("Result: " + result1);
        System.out.println("Current email: " + (currentUser != null ? currentUser.email : "N/A"));
        logout();
        System.out.println("\n-------------------------------------------------\n");

        // Reset user email for next tests
        userDatabase.get("testuser").email = "user@example.com";

        System.out.println("--- Test Case 2: Failure - Not Logged In ---");
        System.out.println("Current user is logged out.");
        String result2 = changeEmail("user@example.com", "another.email@example.com", "Password123!");
        System.out.println("Result: " + result2);
        System.out.println("\n-------------------------------------------------\n");

        System.out.println("--- Test Case 3: Failure - Incorrect Password ---");
        login("testuser", "Password123!");
        System.out.println("Login successful for: " + (currentUser != null ? currentUser.username : "none"));
        String result3 = changeEmail("user@example.com", "another.email@example.com", "WrongPassword!");
        System.out.println("Result: " + result3);
        System.out.println("Email was not changed: " + currentUser.email);
        logout();
        System.out.println("\n-------------------------------------------------\n");

        System.out.println("--- Test Case 4: Failure - Incorrect Old Email ---");
        login("testuser", "Password123!");
        System.out.println("Login successful for: " + (currentUser != null ? currentUser.username : "none"));
        String result4 = changeEmail("wrong.old@example.com", "another.email@example.com", "Password123!");
        System.out.println("Result: " + result4);
        System.out.println("Email was not changed: " + currentUser.email);
        logout();
        System.out.println("\n-------------------------------------------------\n");

        System.out.println("--- Test Case 5: Failure - New Email is Same as Old ---");
        login("testuser", "Password123!");
        System.out.println("Login successful for: " + (currentUser != null ? currentUser.username : "none"));
        String result5 = changeEmail("user@example.com", "user@example.com", "Password123!");
        System.out.println("Result: " + result5);
        System.out.println("Email was not changed: " + currentUser.email);
        logout();
        System.out.println("\n-------------------------------------------------\n");
    }
}