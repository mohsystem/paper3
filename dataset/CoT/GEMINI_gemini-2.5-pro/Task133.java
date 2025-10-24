import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.HexFormat;
import java.util.regex.Pattern;

class Task133 {

    // --- User Data Store ---
    // In a real application, this would be a database.
    private static final Map<String, User> userStore = new HashMap<>();

    // --- Security Configuration ---
    private static final int MIN_PASSWORD_LENGTH = 12;
    // Regex: At least one digit, one lowercase, one uppercase, one special character, and 12+ characters long.
    private static final Pattern PASSWORD_PATTERN = 
        Pattern.compile("^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=!])(?=\\S+$).{" + MIN_PASSWORD_LENGTH + ",}$");
    private static final SecureRandom secureRandom = new SecureRandom();

    // User model class
    private static class User {
        String username;
        String passwordHash; // In a real app, this would be a strong hash (e.g., Argon2, bcrypt)
        String resetToken;

        User(String username, String password) {
            this.username = username;
            // SECURITY: THIS IS NOT A SECURE HASH.
            // For demonstration only. Use a library like Spring Security or Bouncy Castle for proper password hashing.
            this.passwordHash = "hashed_" + password; 
            this.resetToken = null;
        }
    }

    /**
     * SECURITY: This is a placeholder for a real password hashing function.
     * NEVER store passwords in plaintext or with a simple scheme like this.
     * Use a standard, slow, salted hashing algorithm like Argon2, scrypt, or bcrypt.
     * @param password The plaintext password.
     * @return A "hashed" representation of the password.
     */
    private static String hashPassword(String password) {
        return "hashed_" + password;
    }

    /**
     * Validates if a new password meets the security policy.
     * @param password The password to validate.
     * @return true if the password is strong, false otherwise.
     */
    private static boolean isPasswordStrong(String password) {
        if (password == null) {
            return false;
        }
        return PASSWORD_PATTERN.matcher(password).matches();
    }

    /**
     * Generates a cryptographically secure random token.
     * @return A 64-character hex string token.
     */
    private static String generateResetToken() {
        byte[] tokenBytes = new byte[32]; // 256 bits
        secureRandom.nextBytes(tokenBytes);
        return HexFormat.of().formatHex(tokenBytes);
    }

    /**
     * Initiates a password reset request for a user.
     * SECURITY: To prevent user enumeration attacks, this function's observable behavior
     * should be the same whether the user exists or not. In a real application,
     * you would send an email if the user exists and do nothing otherwise, but the
     * response to the caller would be a generic "If an account exists, an email has been sent."
     * For this self-contained example, we return the token for testing purposes.
     * @param username The username requesting a reset.
     * @return A reset token.
     */
    public static String requestPasswordReset(String username) {
        String token = generateResetToken();
        User user = userStore.get(username);
        if (user != null) {
            user.resetToken = token;
        }
        return token; // Returned for demonstration; in reality, this would be emailed.
    }

    /**
     * Resets the user's password if the token is valid and the new password is strong.
     * @param username The user's username.
     * @param token The reset token received from the request.
     * @param newPassword The desired new password.
     * @return true if the password was successfully reset, false otherwise.
     */
    public static boolean resetPassword(String username, String token, String newPassword) {
        // 1. Validate new password strength
        if (!isPasswordStrong(newPassword)) {
            System.out.println("Password reset failed for " + username + ": New password is too weak.");
            return false;
        }

        User user = userStore.get(username);

        // 2. Validate user and token
        if (user == null || user.resetToken == null || !user.resetToken.equals(token)) {
            System.out.println("Password reset failed for " + username + ": Invalid username or token.");
            return false;
        }

        // 3. Update password and invalidate token
        user.passwordHash = hashPassword(newPassword);
        user.resetToken = null; // Invalidate token after use

        System.out.println("Password for " + username + " has been reset successfully.");
        return true;
    }

    public static void main(String[] args) {
        // Pre-populate user store
        userStore.put("alice", new User("alice", "Password123!"));
        userStore.put("bob", new User("bob", "SecurePass456@"));

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Successful Password Reset
        System.out.println("\n--- Test Case 1: Successful Reset ---");
        String aliceToken = requestPasswordReset("alice");
        System.out.println("Alice requested a reset token: " + aliceToken);
        boolean success1 = resetPassword("alice", aliceToken, "NewStrongP@ssw0rd123");
        System.out.println("Result: " + (success1 ? "SUCCESS" : "FAILURE"));

        // Test Case 2: Failed Reset - Invalid Token
        System.out.println("\n--- Test Case 2: Invalid Token ---");
        String bobToken = requestPasswordReset("bob");
        System.out.println("Bob requested a reset token: " + bobToken);
        boolean success2 = resetPassword("bob", "thisIsAWrongToken12345", "AnotherP@ssword1");
        System.out.println("Result: " + (success2 ? "SUCCESS" : "FAILURE"));

        // Test Case 3: Failed Reset - Weak Password
        System.out.println("\n--- Test Case 3: Weak Password ---");
        // Bob's token is still valid
        boolean success3 = resetPassword("bob", bobToken, "weak");
        System.out.println("Result: " + (success3 ? "SUCCESS" : "FAILURE"));

        // Test Case 4: Failed Reset - Non-existent User
        System.out.println("\n--- Test Case 4: Non-existent User ---");
        String charlieToken = requestPasswordReset("charlie"); // Token is generated but not stored
        System.out.println("A token was generated for a potential user 'charlie': " + charlieToken);
        boolean success4 = resetPassword("charlie", charlieToken, "StrongP@ssword123");
        System.out.println("Result: " + (success4 ? "SUCCESS" : "FAILURE"));
        
        // Test Case 5: Failed Reset - Token Reuse
        System.out.println("\n--- Test Case 5: Token Reuse ---");
        // First, a valid reset for bob
        boolean firstReset = resetPassword("bob", bobToken, "BobNewP@ssw0rd987");
        System.out.println("Bob's first reset attempt with valid token: " + (firstReset ? "SUCCESS" : "FAILURE"));
        // Now, try to reuse the same token
        System.out.println("Attempting to reuse Bob's token...");
        boolean success5 = resetPassword("bob", bobToken, "SomeOtherP@ssw0rd");
        System.out.println("Result: " + (success5 ? "SUCCESS" : "FAILURE"));
    }
}