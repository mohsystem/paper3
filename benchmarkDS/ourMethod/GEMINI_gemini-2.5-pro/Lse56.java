import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse56 {

    // In a real application, use a proper database. This is for demonstration.
    private static final Map<String, User> userDatabase = new HashMap<>();

    // Simple email regex for validation
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
            "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$");

    private static class User {
        String username;
        String email;
        String passwordHash;
        byte[] salt;
        boolean isLoggedIn;

        User(String username, String email, String password) {
            this.username = username;
            this.email = email;
            this.salt = generateSalt();
            this.passwordHash = hashPassword(password, this.salt);
            this.isLoggedIn = false;
        }
    }

    /**
     * Generates a cryptographically secure salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password using PBKDF2 with HMAC-SHA256.
     */
    private static String hashPassword(String password, byte[] salt) {
        if (password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Password cannot be empty.");
        }
        try {
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, 65536, 256);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    /**
     * Verifies a password against a stored hash.
     */
    private static boolean verifyPassword(String password, String storedHash, byte[] salt) {
        if (password == null || password.isEmpty()) {
            return false;
        }
        String newHash = hashPassword(password, salt);
        return newHash.equals(storedHash);
    }

    /**
     * Changes the user's email after performing security checks.
     *
     * @param username The username of the user changing their email.
     * @param oldEmail The user's current email for verification.
     * @param newEmail The desired new email.
     * @param password The user's password for verification.
     * @return true if the email was changed successfully, false otherwise.
     */
    public static boolean changeEmail(String username, String oldEmail, String newEmail, String password) {
        // Rule #1: Input Validation
        if (username == null || username.trim().isEmpty() ||
            oldEmail == null || oldEmail.trim().isEmpty() ||
            newEmail == null || newEmail.trim().isEmpty() ||
            password == null || password.isEmpty()) {
            System.out.println("Error: All fields are required.");
            return false;
        }

        if (!EMAIL_PATTERN.matcher(newEmail).matches()) {
            System.out.println("Error: New email format is invalid.");
            return false;
        }

        User user = userDatabase.get(username);

        // Security checks
        if (user == null) {
             // Generic error to prevent user enumeration
            System.out.println("Error: Email change failed.");
            return false;
        }

        if (!user.isLoggedIn) {
            System.out.println("Error: User must be logged in to change email.");
            return false;
        }
        
        if (newEmail.equals(oldEmail)) {
            System.out.println("Error: New email must be different from the old email.");
            return false;
        }
        
        // Constant time comparison for emails is not strictly necessary but good practice
        if (!user.email.equals(oldEmail)) {
            System.out.println("Error: Email change failed."); // Generic error
            return false;
        }

        if (!verifyPassword(password, user.passwordHash, user.salt)) {
            System.out.println("Error: Email change failed."); // Generic error
            return false;
        }

        // All checks passed, update the email
        user.email = newEmail;
        System.out.println("Email for user '" + username + "' has been successfully updated to " + newEmail);
        return true;
    }

    public static void main(String[] args) {
        // Setup: Create a user and log them in
        String testUsername = "testUser";
        String initialEmail = "initial@example.com";
        String userPassword = "Password123!";
        
        User testUser = new User(testUsername, initialEmail, userPassword);
        userDatabase.put(testUsername, testUser);

        System.out.println("Initial state: User '" + testUser.username + "' with email '" + testUser.email + "' is logged out.");
        System.out.println("--------------------------------------------------");

        // --- Test Cases ---
        System.out.println("Running Test Cases...\n");

        // Test Case 1: Successful email change
        System.out.println("--- Test Case 1: Successful Change ---");
        testUser.isLoggedIn = true; // Simulate login
        System.out.println("User is now logged in.");
        boolean result1 = changeEmail(testUsername, initialEmail, "new.email@example.com", userPassword);
        System.out.println("Result: " + (result1 ? "SUCCESS" : "FAILURE"));
        System.out.println("Current email: " + testUser.email + "\n");
        String updatedEmail = testUser.email; // Save for next tests

        // Test Case 2: Failure - User not logged in
        System.out.println("--- Test Case 2: Failure (Not Logged In) ---");
        testUser.isLoggedIn = false; // Simulate logout
        System.out.println("User is now logged out.");
        boolean result2 = changeEmail(testUsername, updatedEmail, "another.email@example.com", userPassword);
        System.out.println("Result: " + (result2 ? "SUCCESS" : "FAILURE"));
        System.out.println("Current email: " + testUser.email + "\n");
        testUser.isLoggedIn = true; // Log back in for subsequent tests

        // Test Case 3: Failure - Incorrect password
        System.out.println("--- Test Case 3: Failure (Incorrect Password) ---");
        boolean result3 = changeEmail(testUsername, updatedEmail, "another.email@example.com", "WrongPassword!");
        System.out.println("Result: " + (result3 ? "SUCCESS" : "FAILURE"));
        System.out.println("Current email: " + testUser.email + "\n");

        // Test Case 4: Failure - Incorrect old email
        System.out.println("--- Test Case 4: Failure (Incorrect Old Email) ---");
        boolean result4 = changeEmail(testUsername, "wrong.old.email@example.com", "another.email@example.com", userPassword);
        System.out.println("Result: " + (result4 ? "SUCCESS" : "FAILURE"));
        System.out.println("Current email: " + testUser.email + "\n");

        // Test Case 5: Failure - New email is same as old
        System.out.println("--- Test Case 5: Failure (New Email is Same as Old) ---");
        boolean result5 = changeEmail(testUsername, updatedEmail, updatedEmail, userPassword);
        System.out.println("Result: " + (result5 ? "SUCCESS" : "FAILURE"));
        System.out.println("Current email: " + testUser.email + "\n");
    }
}