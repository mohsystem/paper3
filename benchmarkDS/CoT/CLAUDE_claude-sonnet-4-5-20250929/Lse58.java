
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse58 {
    private static class User {
        private String email;
        private String passwordHash;
        private String salt;
        private boolean isLoggedIn;

        public User(String email, String password) {
            this.email = email;
            this.salt = generateSalt();
            this.passwordHash = hashPassword(password, this.salt);
            this.isLoggedIn = false;
        }

        private String generateSalt() {
            SecureRandom random = new SecureRandom();
            byte[] saltBytes = new byte[16];
            random.nextBytes(saltBytes);
            return Base64.getEncoder().encodeToString(saltBytes);
        }

        private String hashPassword(String password, String salt) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(salt.getBytes());
                byte[] hashedPassword = md.digest(password.getBytes());
                return Base64.getEncoder().encodeToString(hashedPassword);
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException("Error hashing password", e);
            }
        }

        public boolean login(String email, String password) {
            if (this.email.equals(email) && this.passwordHash.equals(hashPassword(password, this.salt))) {
                this.isLoggedIn = true;
                return true;
            }
            return false;
        }

        public void logout() {
            this.isLoggedIn = false;
        }

        public boolean changeEmail(String oldEmail, String password, String newEmail) {
            if (!isLoggedIn) {
                System.out.println("Error: User must be logged in to change email");
                return false;
            }

            if (newEmail == null || newEmail.trim().isEmpty() || !isValidEmail(newEmail)) {
                System.out.println("Error: Invalid new email format");
                return false;
            }

            if (!this.email.equals(oldEmail)) {
                System.out.println("Error: Old email does not match");
                return false;
            }

            if (!this.passwordHash.equals(hashPassword(password, this.salt))) {
                System.out.println("Error: Incorrect password");
                return false;
            }

            this.email = newEmail;
            System.out.println("Email successfully changed to: " + newEmail);
            return true;
        }

        private boolean isValidEmail(String email) {
            return email.matches("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        }

        public String getEmail() {
            return email;
        }

        public boolean isLoggedIn() {
            return isLoggedIn;
        }
    }

    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Successful email change ===");
        User user1 = new User("user1@example.com", "password123");
        user1.login("user1@example.com", "password123");
        user1.changeEmail("user1@example.com", "password123", "newemail1@example.com");
        System.out.println("Current email: " + user1.getEmail() + "\\n");

        System.out.println("=== Test Case 2: Wrong old email ===");
        User user2 = new User("user2@example.com", "password456");
        user2.login("user2@example.com", "password456");
        user2.changeEmail("wrong@example.com", "password456", "newemail2@example.com");
        System.out.println("Current email: " + user2.getEmail() + "\\n");

        System.out.println("=== Test Case 3: Wrong password ===");
        User user3 = new User("user3@example.com", "password789");
        user3.login("user3@example.com", "password789");
        user3.changeEmail("user3@example.com", "wrongpassword", "newemail3@example.com");
        System.out.println("Current email: " + user3.getEmail() + "\\n");

        System.out.println("=== Test Case 4: Not logged in ===");
        User user4 = new User("user4@example.com", "password000");
        user4.changeEmail("user4@example.com", "password000", "newemail4@example.com");
        System.out.println("Current email: " + user4.getEmail() + "\\n");

        System.out.println("=== Test Case 5: Invalid new email format ===");
        User user5 = new User("user5@example.com", "password111");
        user5.login("user5@example.com", "password111");
        user5.changeEmail("user5@example.com", "password111", "invalidemail");
        System.out.println("Current email: " + user5.getEmail() + "\\n");
    }
}
