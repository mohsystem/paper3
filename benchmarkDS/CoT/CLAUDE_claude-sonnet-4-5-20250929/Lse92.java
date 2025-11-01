
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse92 {
    private static final int SALT_LENGTH = 16;
    private static final int HASH_ITERATIONS = 65536;
    private static final int HASH_LENGTH = 128;
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,20}$");
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final int MAX_PASSWORD_LENGTH = 128;

    public static class RegistrationResult {
        public boolean success;
        public String message;
        public String hashedPassword;
        public String salt;

        public RegistrationResult(boolean success, String message, String hashedPassword, String salt) {
            this.success = success;
            this.message = message;
            this.hashedPassword = hashedPassword;
            this.salt = salt;
        }
    }

    public static RegistrationResult registerUser(String username, String password) {
        // Input validation
        if (username == null || username.trim().isEmpty()) {
            return new RegistrationResult(false, "Username cannot be empty", null, null);
        }

        if (!USERNAME_PATTERN.matcher(username).matches()) {
            return new RegistrationResult(false, "Invalid username format", null, null);
        }

        if (password == null || password.length() < MIN_PASSWORD_LENGTH) {
            return new RegistrationResult(false, "Password too short", null, null);
        }

        if (password.length() > MAX_PASSWORD_LENGTH) {
            return new RegistrationResult(false, "Password too long", null, null);
        }

        try {
            // Generate salt
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH];
            random.nextBytes(salt);

            // Hash password
            String hashedPassword = hashPassword(password, salt);
            String saltBase64 = Base64.getEncoder().encodeToString(salt);

            return new RegistrationResult(true, "Registration successful", hashedPassword, saltBase64);
        } catch (Exception e) {
            return new RegistrationResult(false, "Registration failed", null, null);
        }
    }

    private static String hashPassword(String password, byte[] salt) throws Exception {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, HASH_ITERATIONS, HASH_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] hash = factory.generateSecret(spec).getEncoded();
        return Base64.getEncoder().encodeToString(hash);
    }

    public static void main(String[] args) {
        System.out.println("Registration Form - Test Cases\\n");

        // Test case 1: Valid registration
        RegistrationResult result1 = registerUser("john_doe", "SecurePass123!");
        System.out.println("Test 1 - Valid user:");
        System.out.println("Success: " + result1.success + ", Message: " + result1.message);
        if (result1.success) {
            System.out.println("Hash: " + result1.hashedPassword.substring(0, 20) + "...");
        }

        // Test case 2: Short password
        RegistrationResult result2 = registerUser("alice", "pass");
        System.out.println("\\nTest 2 - Short password:");
        System.out.println("Success: " + result2.success + ", Message: " + result2.message);

        // Test case 3: Invalid username
        RegistrationResult result3 = registerUser("user@name!", "ValidPass123");
        System.out.println("\\nTest 3 - Invalid username:");
        System.out.println("Success: " + result3.success + ", Message: " + result3.message);

        // Test case 4: Empty username
        RegistrationResult result4 = registerUser("", "ValidPass123");
        System.out.println("\\nTest 4 - Empty username:");
        System.out.println("Success: " + result4.success + ", Message: " + result4.message);

        // Test case 5: Valid registration with special characters
        RegistrationResult result5 = registerUser("user_123", "MyP@ssw0rd!2024");
        System.out.println("\\nTest 5 - Valid user with special chars:");
        System.out.println("Success: " + result5.success + ", Message: " + result5.message);
        if (result5.success) {
            System.out.println("Hash: " + result5.hashedPassword.substring(0, 20) + "...");
        }
    }
}
