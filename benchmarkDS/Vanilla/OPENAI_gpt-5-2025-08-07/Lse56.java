import java.util.*;

public class Lse56 {

    static class User {
        private String email;
        private String password;

        public User(String email, String password) {
            this.email = email;
            this.password = password;
        }
        public String getEmail() { return email; }
        public String getPassword() { return password; }
        public void setEmail(String email) { this.email = email; }
    }

    // Function to change email with required validations
    public static String changeEmail(User user, boolean loggedIn, String oldEmail, String newEmail, String confirmPassword) {
        if (user == null || oldEmail == null || newEmail == null || confirmPassword == null) {
            return "ERROR: Invalid input.";
        }
        if (!loggedIn) {
            return "ERROR: User must be logged in.";
        }
        if (!oldEmail.equals(user.getEmail())) {
            return "ERROR: Old email does not match.";
        }
        if (newEmail.equals(user.getEmail())) {
            return "ERROR: New email must be different from old email.";
        }
        if (!confirmPassword.equals(user.getPassword())) {
            return "ERROR: Incorrect password.";
        }
        user.setEmail(newEmail);
        return "SUCCESS: Email changed to " + newEmail;
    }

    public static void main(String[] args) {
        // Test case 1: Success
        User u1 = new User("user@example.com", "secret123");
        System.out.println(changeEmail(u1, true, "user@example.com", "new@example.com", "secret123"));

        // Test case 2: Not logged in
        User u2 = new User("user@example.com", "secret123");
        System.out.println(changeEmail(u2, false, "user@example.com", "newer@example.com", "secret123"));

        // Test case 3: Wrong old email
        User u3 = new User("user@example.com", "secret123");
        System.out.println(changeEmail(u3, true, "wrong@example.com", "newer@example.com", "secret123"));

        // Test case 4: Incorrect password
        User u4 = new User("user@example.com", "secret123");
        System.out.println(changeEmail(u4, true, "user@example.com", "newer@example.com", "badpass"));

        // Test case 5: New email same as old
        User u5 = new User("user@example.com", "secret123");
        System.out.println(changeEmail(u5, true, "user@example.com", "user@example.com", "secret123"));
    }
}