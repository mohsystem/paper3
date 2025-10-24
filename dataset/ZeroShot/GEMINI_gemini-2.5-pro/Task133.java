import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.time.Instant;
import java.time.temporal.ChronoUnit;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Task133 {

    // --- In-memory user store (for simulation) ---
    private static final Map<String, User> userDatabase = new HashMap<>();

    // --- Security Constants ---
    private static final int SALT_LENGTH = 16; // 16 bytes for salt
    private static final int TOKEN_LENGTH = 32; // 32 bytes for a secure token
    private static final int HASH_ITERATIONS = 65536;
    private static final int HASH_KEY_LENGTH = 256; // 256 bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final long TOKEN_VALIDITY_MINUTES = 15;
    private static final int MIN_PASSWORD_LENGTH = 8;
    // Password must contain at least one digit, one lowercase, one uppercase, and one special character.
    private static final Pattern PASSWORD_POLICY = Pattern.compile("^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=!])(?=\\S+$).{8,}$");


    private static class User {
        String username;
        byte[] hashedPassword;
        byte[] salt;
        String resetToken;
        Instant tokenExpiry;

        User(String username, byte[] hashedPassword, byte[] salt) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.salt = salt;
        }
    }

    private static byte[] generateRandomBytes(int length) {
        SecureRandom random = new SecureRandom();
        byte[] bytes = new byte[length];
        random.nextBytes(bytes);
        return bytes;
    }

    private static String generateToken() {
        return Base64.getUrlEncoder().withoutPadding().encodeToString(generateRandomBytes(TOKEN_LENGTH));
    }
    
    private static byte[] hashPassword(char[] password, byte[] salt) {
        PBEKeySpec spec = new PBEKeySpec(password, salt, HASH_ITERATIONS, HASH_KEY_LENGTH);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return skf.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new AssertionError("Error while hashing a password: " + e.getMessage(), e);
        } finally {
            spec.clearPassword();
        }
    }

    /**
     * Initiates the password reset process for a user.
     * In a real application, the returned token would be sent to the user's email.
     * @param username The username of the user requesting a password reset.
     * @return A unique reset token, or null if the user does not exist.
     */
    public static String requestPasswordReset(String username) {
        if (username == null || !userDatabase.containsKey(username)) {
            return null; // Do not reveal if user exists or not for security
        }
        User user = userDatabase.get(username);
        String token = generateToken();
        user.resetToken = token;
        user.tokenExpiry = Instant.now().plus(TOKEN_VALIDITY_MINUTES, ChronoUnit.MINUTES);
        return token;
    }

    /**
     * Resets the user's password if the provided token is valid.
     * @param username    The user's username.
     * @param token       The password reset token.
     * @param newPassword The new password.
     * @return true if the password was successfully reset, false otherwise.
     */
    public static boolean resetPassword(String username, String token, String newPassword) {
        if (username == null || token == null || newPassword == null || !userDatabase.containsKey(username)) {
            return false;
        }

        User user = userDatabase.get(username);

        // 1. Validate token
        if (user.resetToken == null || !user.resetToken.equals(token)) {
            return false; // Invalid token
        }

        // 2. Check token expiration
        if (Instant.now().isAfter(user.tokenExpiry)) {
            user.resetToken = null; // Expired, invalidate it
            return false;
        }
        
        // 3. Invalidate token immediately after first use
        user.resetToken = null;
        user.tokenExpiry = null;

        // 4. Validate new password against policy
        if (newPassword.length() < MIN_PASSWORD_LENGTH || !PASSWORD_POLICY.matcher(newPassword).matches()) {
            return false; // Password does not meet policy
        }

        // 5. Update password
        byte[] newSalt = generateRandomBytes(SALT_LENGTH);
        user.salt = newSalt;
        user.hashedPassword = hashPassword(newPassword.toCharArray(), newSalt);
        
        return true;
    }
    
    // Helper to check password for login simulation
    public static boolean checkPassword(String username, String password) {
        if (!userDatabase.containsKey(username)) return false;
        User user = userDatabase.get(username);
        byte[] attemptHash = hashPassword(password.toCharArray(), user.salt);
        return Arrays.equals(user.hashedPassword, attemptHash);
    }

    public static void main(String[] args) {
        // --- Setup: Create some users ---
        String initialPassAlice = "AlicePass123!";
        byte[] saltAlice = generateRandomBytes(SALT_LENGTH);
        byte[] hashAlice = hashPassword(initialPassAlice.toCharArray(), saltAlice);
        userDatabase.put("alice", new User("alice", hashAlice, saltAlice));

        String initialPassBob = "BobSecure@2023";
        byte[] saltBob = generateRandomBytes(SALT_LENGTH);
        byte[] hashBob = hashPassword(initialPassBob.toCharArray(), saltBob);
        userDatabase.put("bob", new User("bob", hashBob, saltBob));

        System.out.println("--- Running Password Reset Test Cases ---");

        // --- Test Case 1: Successful Reset ---
        System.out.println("\n[Test Case 1: Successful Reset]");
        String token1 = requestPasswordReset("alice");
        System.out.println("Alice requested reset. Token received: " + (token1 != null));
        String newPassword1 = "NewSecurePass!456";
        boolean success1 = resetPassword("alice", token1, newPassword1);
        System.out.println("Password reset attempt with valid token: " + (success1 ? "SUCCESS" : "FAIL"));
        System.out.println("Alice can log in with new password: " + checkPassword("alice", newPassword1));
        System.out.println("Alice cannot log in with old password: " + !checkPassword("alice", initialPassAlice));
        
        // --- Test Case 2: Invalid Token ---
        System.out.println("\n[Test Case 2: Invalid Token]");
        String token2 = requestPasswordReset("bob");
        System.out.println("Bob requested reset. Token received: " + (token2 != null));
        boolean success2 = resetPassword("bob", "invalid-token-string", "SomePassword1!");
        System.out.println("Password reset attempt with invalid token: " + (success2 ? "SUCCESS" : "FAIL"));
        System.out.println("Bob can still log in with old password: " + checkPassword("bob", initialPassBob));

        // --- Test Case 3: Expired Token ---
        System.out.println("\n[Test Case 3: Expired Token]");
        String token3 = requestPasswordReset("bob");
        // Manually expire the token for testing purposes
        userDatabase.get("bob").tokenExpiry = Instant.now().minus(1, ChronoUnit.SECONDS);
        System.out.println("Bob requested reset. Token is now expired.");
        boolean success3 = resetPassword("bob", token3, "AnotherPass!789");
        System.out.println("Password reset attempt with expired token: " + (success3 ? "SUCCESS" : "FAIL"));
        System.out.println("Bob can still log in with old password: " + checkPassword("bob", initialPassBob));

        // --- Test Case 4: Weak Password ---
        System.out.println("\n[Test Case 4: Weak Password]");
        String token4 = requestPasswordReset("alice");
        System.out.println("Alice requested reset again.");
        boolean success4 = resetPassword("alice", token4, "weak");
        System.out.println("Password reset attempt with weak password: " + (success4 ? "SUCCESS" : "FAIL"));
        System.out.println("Alice can still log in with her last valid password: " + checkPassword("alice", newPassword1));

        // --- Test Case 5: Non-Existent User ---
        System.out.println("\n[Test Case 5: Non-Existent User]");
        String token5 = requestPasswordReset("dave");
        System.out.println("Request reset for non-existent user 'dave'. Token received: " + (token5 != null));
        boolean success5 = resetPassword("dave", "any-token", "anyPassword1!");
        System.out.println("Password reset attempt for non-existent user: " + (success5 ? "SUCCESS" : "FAIL"));
    }
}