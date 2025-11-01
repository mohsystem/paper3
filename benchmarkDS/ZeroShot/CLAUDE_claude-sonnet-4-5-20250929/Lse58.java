
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse58 {
    private static class User {
        private String username;
        private String email;
        private String passwordHash;
        private boolean loggedIn;

        public User(String username, String email, String password) {
            this.username = username;
            this.email = email;
            this.passwordHash = hashPassword(password);
            this.loggedIn = false;
        }

        private String hashPassword(String password) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                byte[] hash = md.digest(password.getBytes());
                StringBuilder hexString = new StringBuilder();
                for (byte b : hash) {
                    String hex = Integer.toHexString(0xff & b);
                    if (hex.length() == 1) hexString.append('0');
                    hexString.append(hex);
                }
                return hexString.toString();
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException(e);
            }
        }

        public boolean login(String password) {
            if (hashPassword(password).equals(this.passwordHash)) {
                this.loggedIn = true;
                return true;
            }
            return false;
        }

        public void logout() {
            this.loggedIn = false;
        }

        public boolean changeEmail(String oldEmail, String password, String newEmail) {
            if (!loggedIn) {
                return false;
            }
            if (!this.email.equals(oldEmail)) {
                return false;
            }
            if (!hashPassword(password).equals(this.passwordHash)) {
                return false;
            }
            if (newEmail == null || newEmail.trim().isEmpty() || !isValidEmail(newEmail)) {
                return false;
            }
            this.email = newEmail;
            return true;
        }

        private boolean isValidEmail(String email) {
            return email.matches("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        }

        public String getEmail() {
            return email;
        }

        public String getUsername() {
            return username;
        }

        public boolean isLoggedIn() {
            return loggedIn;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Successful email change
        User user1 = new User("john_doe", "john@example.com", "password123");
        user1.login("password123");
        boolean result1 = user1.changeEmail("john@example.com", "password123", "newemail@example.com");
        System.out.println("Test 1 - Successful change: " + result1 + ", New email: " + user1.getEmail());

        // Test Case 2: Wrong old email
        User user2 = new User("jane_doe", "jane@example.com", "securepass");
        user2.login("securepass");
        boolean result2 = user2.changeEmail("wrong@example.com", "securepass", "newemail@example.com");
        System.out.println("Test 2 - Wrong old email: " + result2 + ", Email unchanged: " + user2.getEmail());

        // Test Case 3: Wrong password
        User user3 = new User("bob_smith", "bob@example.com", "mypassword");
        user3.login("mypassword");
        boolean result3 = user3.changeEmail("bob@example.com", "wrongpassword", "newemail@example.com");
        System.out.println("Test 3 - Wrong password: " + result3 + ", Email unchanged: " + user3.getEmail());

        // Test Case 4: Not logged in
        User user4 = new User("alice_wonder", "alice@example.com", "alicepass");
        boolean result4 = user4.changeEmail("alice@example.com", "alicepass", "newemail@example.com");
        System.out.println("Test 4 - Not logged in: " + result4 + ", Email unchanged: " + user4.getEmail());

        // Test Case 5: Invalid new email format
        User user5 = new User("charlie_brown", "charlie@example.com", "charpass");
        user5.login("charpass");
        boolean result5 = user5.changeEmail("charlie@example.com", "charpass", "invalidemail");
        System.out.println("Test 5 - Invalid email format: " + result5 + ", Email unchanged: " + user5.getEmail());
    }
}
