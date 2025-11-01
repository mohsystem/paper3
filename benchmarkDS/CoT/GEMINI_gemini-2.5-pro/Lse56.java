import java.util.Objects;

// In a real application, never store passwords in plain text.
// Always use a strong, salted hashing algorithm like Argon2, scrypt, or bcrypt.
class User {
    String email;
    String password; // Plain text for demonstration only.
    boolean isLoggedIn;

    public User(String email, String password) {
        this.email = email;
        this.password = password;
        this.isLoggedIn = false; // User is logged out by default
    }

    public void login(String password) {
        if (this.password.equals(password)) {
            this.isLoggedIn = true;
            System.out.println("User " + this.email + " logged in successfully.");
        } else {
            System.out.println("Login failed for user " + this.email);
        }
    }

    public void logout() {
        this.isLoggedIn = false;
        System.out.println("User " + this.email + " logged out.");
    }
}

public class Lse56 {

    /**
     * Attempts to change a user's email address after performing security checks.
     *
     * @param user The user object attempting the change.
     * @param oldEmail The user's claimed old email address.
     * @param newEmail The desired new email address.
     * @param password The user's password for confirmation.
     * @return A string indicating success or the specific reason for failure.
     */
    public static String changeEmail(User user, String oldEmail, String newEmail, String password) {
        // 1. Check if the user is logged in
        if (!user.isLoggedIn) {
            return "Error: User is not logged in.";
        }

        // 2. Check if the provided old email matches the current one
        if (!user.email.equals(oldEmail)) {
            return "Error: Old email does not match the current email.";
        }

        // 3. Check if the provided password is correct
        if (!user.password.equals(password)) {
            return "Error: Incorrect password.";
        }

        // 4. Check if the new email is different from the old email
        if (user.email.equals(newEmail)) {
            return "Error: New email cannot be the same as the old email.";
        }
        
        // In a real application, you would also validate the new email format.

        // All checks passed, update the email
        user.email = newEmail;
        return "Success: Email has been changed to " + newEmail;
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Successful Email Change
        System.out.println("\n--- Test Case 1: Successful Change ---");
        User user1 = new User("test1@example.com", "password123");
        user1.login("password123");
        String result1 = changeEmail(user1, "test1@example.com", "new1@example.com", "password123");
        System.out.println("Result: " + result1);
        System.out.println("User's current email: " + user1.email);
        user1.logout();

        // Test Case 2: User Not Logged In
        System.out.println("\n--- Test Case 2: User Not Logged In ---");
        User user2 = new User("test2@example.com", "password456");
        // user2 does not log in
        String result2 = changeEmail(user2, "test2@example.com", "new2@example.com", "password456");
        System.out.println("Result: " + result2);
        System.out.println("User's current email: " + user2.email);

        // Test Case 3: Incorrect Old Email
        System.out.println("\n--- Test Case 3: Incorrect Old Email ---");
        User user3 = new User("test3@example.com", "password789");
        user3.login("password789");
        String result3 = changeEmail(user3, "wrong-email@example.com", "new3@example.com", "password789");
        System.out.println("Result: " + result3);
        System.out.println("User's current email: " + user3.email);
        user3.logout();

        // Test Case 4: Incorrect Password
        System.out.println("\n--- Test Case 4: Incorrect Password ---");
        User user4 = new User("test4@example.com", "securepass");
        user4.login("securepass");
        String result4 = changeEmail(user4, "test4@example.com", "new4@example.com", "wrongpass");
        System.out.println("Result: " + result4);
        System.out.println("User's current email: " + user4.email);
        user4.logout();

        // Test Case 5: New Email is Same as Old Email
        System.out.println("\n--- Test Case 5: New Email is Same as Old Email ---");
        User user5 = new User("test5@example.com", "pass12345");
        user5.login("pass12345");
        String result5 = changeEmail(user5, "test5@example.com", "test5@example.com", "pass12345");
        System.out.println("Result: " + result5);
        System.out.println("User's current email: " + user5.email);
        user5.logout();
    }
}