import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Objects;

public class Lse57 {

    // --- Data Structures to Simulate Database and Sessions ---

    // Simulates a table of users, mapping username to User object.
    private static final Map<String, User> userDatabase = new HashMap<>();
    // Simulates a unique index on email for fast lookups.
    private static final Set<String> emailDatabase = new HashSet<>();
    // Simulates a set of currently active user sessions.
    private static final Set<String> loggedInUsers = new HashSet<>();

    // --- User Model ---
    private static class User {
        String username;
        String email;
        String hashedPassword; // In a real app, this would be a strong, salted hash.

        User(String username, String email, String hashedPassword) {
            this.username = username;
            this.email = email;
            this.hashedPassword = hashedPassword;
        }

        @Override
        public String toString() {
            return "User{" +
                   "username='" + username + '\'' +
                   ", email='" + email + '\'' +
                   '}';
        }
    }

    // --- Security Simulation ---

    /**
     * A simple, insecure password hashing simulation.
     * WARNING: Do NOT use this in production. Use a strong algorithm like Argon2, scrypt, or bcrypt.
     */
    private static String hashPassword(String password) {
        if (password == null) return "";
        // Simple "hash" for demonstration: reverse the string and append a static "salt"
        return new StringBuilder(password).reverse().toString() + "_hashed";
    }

    // --- Core Logic ---

    /**
     * Populates the simulated database with initial data.
     */
    private static void setupDatabase() {
        userDatabase.clear();
        emailDatabase.clear();
        loggedInUsers.clear();

        String[] usernames = {"alice", "bob", "charlie"};
        String[] emails = {"alice@example.com", "bob@example.com", "charlie@example.com"};
        String[] passwords = {"password123", "bobpass", "securepass"};

        for (int i = 0; i < usernames.length; i++) {
            String hashedPassword = hashPassword(passwords[i]);
            User user = new User(usernames[i], emails[i], hashedPassword);
            userDatabase.put(usernames[i], user);
            emailDatabase.add(emails[i]);
        }
    }

    /**
     * Simulates a user logging in.
     */
    public static boolean login(String username, String password) {
        User user = userDatabase.get(username);
        if (user != null && user.hashedPassword.equals(hashPassword(password))) {
            loggedInUsers.add(username);
            return true;
        }
        return false;
    }

    /**
     * Simulates a user logging out.
     */
    public static void logout(String username) {
        loggedInUsers.remove(username);
    }

    /**
     * Allows a logged-in user to change their email after confirming their identity.
     *
     * @param loggedInUsername The username of the currently logged-in user.
     * @param oldEmail         The user's current email address, for confirmation.
     * @param newEmail         The new email address to set.
     * @param confirmPassword  The user's current password, for verification.
     * @return A status message indicating success or failure.
     */
    public static String changeEmail(String loggedInUsername, String oldEmail, String newEmail, String confirmPassword) {
        // 1. Check if user is logged in
        if (!loggedInUsers.contains(loggedInUsername)) {
            return "Error: User is not logged in.";
        }

        // 2. Fetch user data
        User user = userDatabase.get(loggedInUsername);
        if (user == null) {
            // This case should not be reachable if login logic is sound
            return "Error: User data not found for logged-in user.";
        }

        // 3. Verify old email matches records for this user
        if (!user.email.equals(oldEmail)) {
            return "Error: The provided old email does not match our records.";
        }
        
        // 4. Verify password
        if (!user.hashedPassword.equals(hashPassword(confirmPassword))) {
            return "Error: Incorrect password.";
        }

        // 5. Validate new email
        if (newEmail == null || newEmail.trim().isEmpty() || !newEmail.contains("@")) {
            return "Error: New email format is invalid.";
        }
        if (user.email.equals(newEmail)) {
            return "Error: New email cannot be the same as the old email.";
        }
        if (emailDatabase.contains(newEmail)) {
            return "Error: New email is already in use by another account.";
        }

        // 6. All checks passed, perform the update
        emailDatabase.remove(user.email);
        user.email = newEmail;
        emailDatabase.add(newEmail);
        userDatabase.put(loggedInUsername, user);

        return "Success: Email has been changed to " + newEmail;
    }

    public static void main(String[] args) {
        // Test Case 1: Successful email change
        System.out.println("--- Test Case 1: Successful Change ---");
        setupDatabase();
        login("alice", "password123");
        System.out.println("Alice's current info: " + userDatabase.get("alice"));
        String result1 = changeEmail("alice", "alice@example.com", "alice_new@example.com", "password123");
        System.out.println("Result: " + result1);
        System.out.println("Alice's updated info: " + userDatabase.get("alice"));
        logout("alice");
        System.out.println();

        // Test Case 2: Failed change due to incorrect password
        System.out.println("--- Test Case 2: Incorrect Password ---");
        setupDatabase();
        login("bob", "bobpass");
        String result2 = changeEmail("bob", "bob@example.com", "bob_new@example.com", "wrongpassword");
        System.out.println("Result: " + result2);
        System.out.println("Bob's info (should be unchanged): " + userDatabase.get("bob"));
        logout("bob");
        System.out.println();
        
        // Test Case 3: Failed change because user is not logged in
        System.out.println("--- Test Case 3: User Not Logged In ---");
        setupDatabase();
        // Note: No login call for charlie
        String result3 = changeEmail("charlie", "charlie@example.com", "charlie_new@example.com", "securepass");
        System.out.println("Result: " + result3);
        System.out.println();

        // Test Case 4: Failed change because new email is already taken
        System.out.println("--- Test Case 4: New Email Already Exists ---");
        setupDatabase();
        login("alice", "password123");
        String result4 = changeEmail("alice", "alice@example.com", "bob@example.com", "password123");
        System.out.println("Result: " + result4);
        logout("alice");
        System.out.println();

        // Test Case 5: Failed change due to incorrect old email provided
        System.out.println("--- Test Case 5: Incorrect Old Email Provided ---");
        setupDatabase();
        login("bob", "bobpass");
        String result5 = changeEmail("bob", "wrong_old@example.com", "bob_new@example.com", "bobpass");
        System.out.println("Result: " + result5);
        logout("bob");
        System.out.println();
    }
}