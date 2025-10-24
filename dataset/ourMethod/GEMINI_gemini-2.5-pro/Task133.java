import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;
import java.security.MessageDigest;

public class Task133 {

    private static final int SALT_LENGTH = 16;
    private static final int TOKEN_LENGTH_BYTES = 32;
    private static final int PBKDF2_ITERATIONS = 310000;
    private static final int HASH_LENGTH = 256;
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final long TOKEN_VALIDITY_SECONDS = 300; // 5 minutes

    private static final SecureRandom secureRandom = new SecureRandom();
    
    // In-memory storage for demonstration purposes. In a real application, use a secure database.
    private static final Map<String, User> userDatabase = new HashMap<>();
    private static final Map<String, ResetToken> tokenStore = new HashMap<>();

    private static class User {
        byte[] hashedPassword;
        byte[] salt;

        User(byte[] hashedPassword, byte[] salt) {
            this.hashedPassword = hashedPassword;
            this.salt = salt;
        }
    }

    private static class ResetToken {
        byte[] hashedToken;
        byte[] salt;
        Instant expiration;

        ResetToken(byte[] hashedToken, byte[] salt, Instant expiration) {
            this.hashedToken = hashedToken;
            this.salt = salt;
            this.expiration = expiration;
        }
    }
    
    private static byte[] generateSalt() {
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        return salt;
    }

    private static byte[] hash(char[] data, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(data, salt, PBKDF2_ITERATIONS, HASH_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing data", e);
        }
    }

    private static boolean isPasswordStrong(String password) {
        if (password == null || password.length() < 12) {
            return false;
        }
        // Requires at least one digit, one lowercase, one uppercase, one special character
        Pattern pattern = Pattern.compile("^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[!@#&()–[{}]:;',?/*~$^+=<>]).{12,}$");
        return pattern.matcher(password).matches();
    }
    
    // This is a simplified user creation for the demo.
    public static boolean createUser(String username, String password) {
        if (username == null || username.isEmpty() || userDatabase.containsKey(username)) {
            return false;
        }
        if (!isPasswordStrong(password)) {
            return false;
        }
        byte[] salt = generateSalt();
        byte[] hashedPassword = hash(password.toCharArray(), salt);
        userDatabase.put(username, new User(hashedPassword, salt));
        return true;
    }

    public static String requestPasswordReset(String username) {
        if (username == null || !userDatabase.containsKey(username)) {
            return null; // User not found
        }

        byte[] tokenBytes = new byte[TOKEN_LENGTH_BYTES];
        secureRandom.nextBytes(tokenBytes);
        String plainTextToken = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);

        byte[] tokenSalt = generateSalt();
        byte[] hashedToken = hash(plainTextToken.toCharArray(), tokenSalt);
        Instant expiration = Instant.now().plusSeconds(TOKEN_VALIDITY_SECONDS);

        tokenStore.put(username, new ResetToken(hashedToken, tokenSalt, expiration));

        return plainTextToken;
    }

    public static boolean resetPassword(String username, String token, String newPassword) {
        if (username == null || token == null || newPassword == null || !userDatabase.containsKey(username)) {
            return false;
        }

        ResetToken storedToken = tokenStore.get(username);
        if (storedToken == null) {
            return false; // No pending reset request
        }

        if (Instant.now().isAfter(storedToken.expiration)) {
            tokenStore.remove(username); // Clean up expired token
            return false; // Token expired
        }

        byte[] providedTokenHash = hash(token.toCharArray(), storedToken.salt);

        // Use constant-time comparison to prevent timing attacks
        if (!MessageDigest.isEqual(providedTokenHash, storedToken.hashedToken)) {
            return false; // Invalid token
        }

        if (!isPasswordStrong(newPassword)) {
            return false; // New password does not meet policy
        }

        // All checks passed, update password and invalidate token
        byte[] newSalt = generateSalt();
        byte[] newHashedPassword = hash(newPassword.toCharArray(), newSalt);
        userDatabase.put(username, new User(newHashedPassword, newSalt));
        
        tokenStore.remove(username); // Invalidate token after use

        return true;
    }
    
    public static void main(String[] args) {
        System.out.println("Running password reset tests...");
        String username = "testuser";
        String initialPassword = "Password!12345";
        String newPassword = "NewPassword@54321";

        // Setup: Create a user
        if (createUser(username, initialPassword)) {
            System.out.println("1. User '" + username + "' created successfully.");
        } else {
            System.out.println("1. Failed to create user.");
            return;
        }

        // Test Case 1: Successful password reset
        System.out.println("\n--- Test Case 1: Successful Reset ---");
        String token1 = requestPasswordReset(username);
        System.out.println("Generated reset token: " + token1);
        boolean success1 = resetPassword(username, token1, newPassword);
        System.out.println("Reset result: " + (success1 ? "SUCCESS" : "FAILURE"));
        if (!success1) System.out.println("Expected SUCCESS");

        // Test Case 2: Attempt to reuse the token
        System.out.println("\n--- Test Case 2: Reuse Token (Should Fail) ---");
        boolean success2 = resetPassword(username, token1, "AnotherPassword$99");
        System.out.println("Reset result: " + (success2 ? "SUCCESS" : "FAILURE"));
        if (success2) System.out.println("Expected FAILURE");

        // Test Case 3: Attempt to reset with a wrong token
        System.out.println("\n--- Test Case 3: Wrong Token (Should Fail) ---");
        String token3 = requestPasswordReset(username);
        System.out.println("Generated new reset token: " + token3);
        boolean success3 = resetPassword(username, "wrongTokenAbc123", newPassword);
        System.out.println("Reset result: " + (success3 ? "SUCCESS" : "FAILURE"));
        if (success3) System.out.println("Expected FAILURE");

        // Test Case 4: Attempt to reset with a weak password
        System.out.println("\n--- Test Case 4: Weak New Password (Should Fail) ---");
        String token4 = requestPasswordReset(username); // Request a valid token
        boolean success4 = resetPassword(username, token4, "weak");
        System.out.println("Reset result for weak password: " + (success4 ? "SUCCESS" : "FAILURE"));
        if (success4) System.out.println("Expected FAILURE");

        // Test Case 5: Expired token (simulated by setting a short validity period)
        System.out.println("\n--- Test Case 5: Expired Token (Should Fail) ---");
        // For a real test, you would need to adjust TOKEN_VALIDITY_SECONDS to be short (e.g., 1)
        // and add a Thread.sleep(2000). Here, we will just conceptually note it.
        // String token5 = requestPasswordReset(username);
        // try { TimeUnit.SECONDS.sleep(TOKEN_VALIDITY_SECONDS + 1); } catch (InterruptedException e) {}
        // boolean success5 = resetPassword(username, token5, newPassword);
        // System.out.println("Reset result for expired token: " + (success5 ? "SUCCESS" : "FAILURE"));
        System.out.println("Simulating expired token test. In a real scenario, with a short expiry and a wait, this would fail.");
    }
}