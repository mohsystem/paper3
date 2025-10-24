import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task46 {

    // In-memory "database" to store user data
    private static final Map<String, User> userDatabase = new HashMap<>();

    // Secure constants for password hashing
    private static final int SALT_LENGTH = 16; // in bytes
    private static final int ITERATION_COUNT = 65536;
    private static final int KEY_LENGTH = 128; // in bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA1";

    // Regex for validation
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-zA-Z0-9.-]+$"
    );
    // Password must be at least 8 chars, one uppercase, one lowercase, one digit
    private static final Pattern PASSWORD_PATTERN = Pattern.compile(
        "^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z]).{8,}$"
    );

    // User data structure
    private static class User {
        String name;
        String email;
        String salt;
        String hashedPassword;

        User(String name, String email, String salt, String hashedPassword) {
            this.name = name;
            this.email = email;
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }

        @Override
        public String toString() {
            return "User{" +
                   "name='" + name + '\'' +
                   ", email='" + email + '\'' +
                   ", salt='" + salt + '\'' +
                   ", hashedPassword='" + hashedPassword + '\'' +
                   '}';
        }
    }

    /**
     * Generates a cryptographically secure random salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with a given salt.
     */
    public static String hashPassword(String password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    /**
     * Validates user input against security and format policies.
     *
     * @return null if validation passes, otherwise an error message string.
     */
    public static String validateInput(String name, String email, String password) {
        if (name == null || name.trim().isEmpty() || name.length() > 50) {
            return "Invalid name. Must not be empty and be less than 50 characters.";
        }
        if (email == null || !EMAIL_PATTERN.matcher(email).matches()) {
            return "Invalid email format.";
        }
        if (password == null || !PASSWORD_PATTERN.matcher(password).matches()) {
            return "Password is too weak. Must be at least 8 characters, with one uppercase, one lowercase, and one number.";
        }
        return null;
    }

    /**
     * Registers a new user if the input is valid and the user does not already exist.
     *
     * @return A message indicating success or failure.
     */
    public static String registerUser(String name, String email, String password) {
        String validationError = validateInput(name, email, password);
        if (validationError != null) {
            return "Registration failed: " + validationError;
        }

        // Ensure email is case-insensitive for uniqueness
        String normalizedEmail = email.toLowerCase();

        if (userDatabase.containsKey(normalizedEmail)) {
            return "Registration failed: Email '" + email + "' already exists.";
        }

        // Generate salt and hash the password
        byte[] saltBytes = generateSalt();
        String hashedPassword = hashPassword(password, saltBytes);
        String saltString = Base64.getEncoder().encodeToString(saltBytes);

        // Store user in the "database"
        User newUser = new User(name, normalizedEmail, saltString, hashedPassword);
        userDatabase.put(normalizedEmail, newUser);

        return "User '" + name + "' registered successfully.";
    }

    public static void main(String[] args) {
        System.out.println("--- User Registration System ---");

        // Test Case 1: Successful registration
        String result1 = registerUser("Alice", "alice@example.com", "Password123");
        System.out.println("Test 1: " + result1);

        // Test Case 2: Attempt to register with a duplicate email
        String result2 = registerUser("Alice Smith", "alice@example.com", "AnotherPass456");
        System.out.println("Test 2: " + result2);

        // Test Case 3: Registration with an invalid email format
        String result3 = registerUser("Bob", "bob-at-example.com", "SecurePass789");
        System.out.println("Test 3: " + result3);

        // Test Case 4: Registration with a weak password
        String result4 = registerUser("Charlie", "charlie@example.com", "pass");
        System.out.println("Test 4: " + result4);

        // Test Case 5: Another successful registration
        String result5 = registerUser("David", "david@example.com", "MyP@ssw0rd!");
        System.out.println("Test 5: " + result5);

        System.out.println("\n--- Current Database State ---");
        userDatabase.forEach((email, user) -> System.out.println(user));
    }
}