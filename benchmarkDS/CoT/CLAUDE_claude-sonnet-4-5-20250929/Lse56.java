
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse56 {
    private static Map<String, User> users = new HashMap<>();
    private static User currentLoggedInUser = null;
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"
    );

    static class User {
        String email;
        String passwordHash;
        String salt;

        User(String email, String password) {
            this.email = email;
            this.salt = generateSalt();
            this.passwordHash = hashPassword(password, this.salt);
        }
    }

    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }

    private static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }

    private static boolean verifyPassword(String password, String hash, String salt) {
        String newHash = hashPassword(password, salt);
        return MessageDigest.isEqual(newHash.getBytes(), hash.getBytes());
    }

    private static boolean isValidEmail(String email) {
        return email != null && EMAIL_PATTERN.matcher(email).matches();
    }

    public static boolean login(String email, String password) {
        if (!isValidEmail(email) || password == null || password.isEmpty()) {
            return false;
        }

        User user = users.get(email);
        if (user != null && verifyPassword(password, user.passwordHash, user.salt)) {
            currentLoggedInUser = user;
            return true;
        }
        return false;
    }

    public static void logout() {
        currentLoggedInUser = null;
    }

    public static boolean isLoggedIn() {
        return currentLoggedInUser != null;
    }

    public static String changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        if (!isLoggedIn()) {
            return "Error: User must be logged in to change email";
        }

        if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) {
            return "Error: Invalid email format";
        }

        if (confirmPassword == null || confirmPassword.isEmpty()) {
            return "Error: Password is required";
        }

        if (!currentLoggedInUser.email.equals(oldEmail)) {
            return "Error: Old email does not match current user email";
        }

        if (oldEmail.equals(newEmail)) {
            return "Error: New email must be different from old email";
        }

        if (!verifyPassword(confirmPassword, currentLoggedInUser.passwordHash, currentLoggedInUser.salt)) {
            return "Error: Incorrect password";
        }

        if (users.containsKey(newEmail)) {
            return "Error: New email already exists";
        }

        users.remove(oldEmail);
        currentLoggedInUser.email = newEmail;
        users.put(newEmail, currentLoggedInUser);

        return "Success: Email changed successfully";
    }

    public static void main(String[] args) {
        // Test Case 1: Register and login successfully
        users.put("user@example.com", new User("user@example.com", "SecurePass123"));
        System.out.println("Test 1 - Login: " + login("user@example.com", "SecurePass123"));

        // Test Case 2: Change email successfully
        System.out.println("Test 2 - Change Email: " + 
            changeEmail("user@example.com", "newemail@example.com", "SecurePass123"));

        // Test Case 3: Try to change email with wrong password
        logout();
        login("newemail@example.com", "SecurePass123");
        System.out.println("Test 3 - Wrong Password: " + 
            changeEmail("newemail@example.com", "another@example.com", "WrongPass"));

        // Test Case 4: Try to change email to same email
        System.out.println("Test 4 - Same Email: " + 
            changeEmail("newemail@example.com", "newemail@example.com", "SecurePass123"));

        // Test Case 5: Try to change email without being logged in
        logout();
        System.out.println("Test 5 - Not Logged In: " + 
            changeEmail("newemail@example.com", "test@example.com", "SecurePass123"));
    }
}
