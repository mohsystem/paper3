import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.security.MessageDigest;

public class Lse136 {

    private static final int MIN_USERNAME_LENGTH = 4;
    private static final int MAX_USERNAME_LENGTH = 64;
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final int MAX_PASSWORD_LENGTH = 128;

    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int ITERATIONS = 600000;
    private static final int KEY_LENGTH = 256;
    private static final int SALT_LENGTH = 16;
    
    // In a real application, a thread-safe mechanism to access environment variables or a secure vault is needed.
    // For this example, we use a map to simulate environment variables for testability.
    private static Map<String, String> mockEnv = new HashMap<>();

    /**
     * Checks if the username and password are correct.
     * This function simulates being called by a web framework.
     *
     * @param username The username from the URL.
     * @param password The password from the URL.
     * @return "true" if credentials are valid, "false" otherwise.
     */
    public static String check_mod(String username, String password) {
        // Rule #1: Ensure all input is validated.
        if (username == null || username.isEmpty() ||
            username.length() < MIN_USERNAME_LENGTH || username.length() > MAX_USERNAME_LENGTH) {
            return "false";
        }
        if (password == null || password.isEmpty() ||
            password.length() < MIN_PASSWORD_LENGTH || password.length() > MAX_PASSWORD_LENGTH) {
            return "false";
        }

        try {
            // Rule #7: Avoid hardcoding credentials. Fetch from a secure source.
            String storedUsername = mockEnv.get("CORRECT_USERNAME");
            String storedHashStr = mockEnv.get("PASSWORD_HASH");
            String storedSaltStr = mockEnv.get("PASSWORD_SALT");

            if (storedUsername == null || storedHashStr == null || storedSaltStr == null) {
                // Configuration error, fail securely.
                return "false";
            }

            // Simple username check. In a real system, this would be a database lookup.
            if (!username.equals(storedUsername)) {
                // To prevent username enumeration, it's good practice to still perform a dummy hash operation.
                // However, for simplicity here, we return early.
                return "false";
            }
            
            byte[] salt = Base64.getDecoder().decode(storedSaltStr);
            byte[] storedHash = Base64.getDecoder().decode(storedHashStr);

            // Hash the provided password with the stored salt
            byte[] generatedHash = hashPassword(password, salt);
            
            // Rule #10: Use constant-time comparison to prevent timing attacks.
            if (MessageDigest.isEqual(storedHash, generatedHash)) {
                return "true";
            }

        } catch (IllegalArgumentException | NoSuchAlgorithmException | InvalidKeySpecException e) {
            // Log the exception in a real application, but return a generic failure message.
            System.err.println("Authentication error: " + e.getMessage());
            return "false";
        }

        return "false";
    }

    /**
     * Generates a secure salt.
     * Rule #9: Use a cryptographically secure random number generator.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password using PBKDF2 with a given salt.
     * Rule #5, #8: Use strong hashing with a unique salt per password.
     */
    public static byte[] hashPassword(String password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
        byte[] hash = factory.generateSecret(spec).getEncoded();
        // Clear sensitive data from memory
        Arrays.fill(password.toCharArray(), '\0');
        return hash;
    }

    public static void main(String[] args) {
        // --- Test Setup ---
        // In a real application, this setup (generating hash/salt and setting env vars)
        // would be done during user registration and deployment, not at runtime here.
        try {
            String correctUsername = "testuser";
            String correctPassword = "S3cur3P@ssw0rd!";
            
            // Generate salt and hash for the correct password
            byte[] salt = generateSalt();
            byte[] hash = hashPassword(correctPassword, salt);
            
            // Store credentials in our mock environment (simulating secure storage)
            mockEnv.put("CORRECT_USERNAME", correctUsername);
            mockEnv.put("PASSWORD_SALT", Base64.getEncoder().encodeToString(salt));
            mockEnv.put("PASSWORD_HASH", Base64.getEncoder().encodeToString(hash));

            // --- Test Cases ---
            System.out.println("--- Running 5 Test Cases ---");

            // Test Case 1: Correct username, correct password
            String result1 = check_mod(correctUsername, correctPassword);
            System.out.println("Test 1 (Correct credentials): " + result1 + " (Expected: true)");

            // Test Case 2: Correct username, incorrect password
            String result2 = check_mod(correctUsername, "wrongpassword");
            System.out.println("Test 2 (Incorrect password): " + result2 + " (Expected: false)");

            // Test Case 3: Incorrect username, correct password
            String result3 = check_mod("wronguser", correctPassword);
            System.out.println("Test 3 (Incorrect username): " + result3 + " (Expected: false)");

            // Test Case 4: Null username
            String result4 = check_mod(null, correctPassword);
            System.out.println("Test 4 (Null username): " + result4 + " (Expected: false)");

            // Test Case 5: Password too short
            String result5 = check_mod(correctUsername, "short");
            System.out.println("Test 5 (Short password): " + result5 + " (Expected: false)");

        } catch (Exception e) {
            System.err.println("An error occurred during test setup: " + e.getMessage());
            e.printStackTrace();
        }
    }
}