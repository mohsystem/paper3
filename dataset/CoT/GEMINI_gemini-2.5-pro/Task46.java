import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

class User {
    String name;
    String email;
    byte[] passwordHash;
    byte[] salt;

    public User(String name, String email, byte[] passwordHash, byte[] salt) {
        this.name = name;
        this.email = email;
        this.passwordHash = passwordHash;
        this.salt = salt;
    }

    @Override
    public String toString() {
        return "User{" +
                "name='" + name + '\'' +
                ", email='" + email + '\'' +
                '}';
    }
}

public class Task46 {

    // In-memory map to act as a database
    private static final Map<String, User> userDatabase = new HashMap<>();
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$");
    private static final int MIN_PASSWORD_LENGTH = 8;

    /**
     * Generates a salt for password hashing.
     * @return A byte array containing the salt.
     */
    private static byte[] getSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using SHA-512 with a salt.
     * NOTE: In a production environment, use a stronger key derivation function
     * like Argon2, scrypt, or PBKDF2.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-512");
            md.update(salt);
            return md.digest(password.getBytes(StandardCharsets.UTF_8));
        } catch (NoSuchAlgorithmException e) {
            // This should never happen for SHA-512
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a new user after validating the inputs.
     * @param name The user's name.
     * @param email The user's email.
     * @param password The user's password.
     * @return A string indicating the result of the registration attempt.
     */
    public static String registerUser(String name, String email, String password) {
        // 1. Input Validation
        if (name == null || name.trim().isEmpty()) {
            return "Registration failed: Name cannot be empty.";
        }
        if (email == null || !EMAIL_PATTERN.matcher(email).matches()) {
            return "Registration failed: Invalid email format.";
        }
        if (password == null || password.length() < MIN_PASSWORD_LENGTH) {
            return "Registration failed: Password must be at least " + MIN_PASSWORD_LENGTH + " characters long.";
        }
        if (userDatabase.containsKey(email)) {
            return "Registration failed: Email already exists.";
        }

        // 2. Secure Password Handling
        byte[] salt = getSalt();
        byte[] hashedPassword = hashPassword(password, salt);

        // 3. Store User Information
        User newUser = new User(name, email, hashedPassword, salt);
        userDatabase.put(email, newUser);
        
        // Clear sensitive data from memory after use
        Arrays.fill(hashedPassword, (byte) 0);

        return "User '" + name + "' registered successfully.";
    }

    public static void main(String[] args) {
        System.out.println("--- User Registration System Test Cases ---");

        // Test Case 1: Successful registration
        System.out.println("1. " + registerUser("Alice Smith", "alice@example.com", "Password123!"));

        // Test Case 2: Attempt to register with an existing email
        System.out.println("2. " + registerUser("Alice Jones", "alice@example.com", "AnotherPassword456"));

        // Test Case 3: Attempt to register with an invalid email format
        System.out.println("3. " + registerUser("Bob Johnson", "bob-at-invalid", "SecurePass789"));

        // Test Case 4: Attempt to register with a weak (short) password
        System.out.println("4. " + registerUser("Charlie Brown", "charlie@example.com", "pass"));

        // Test Case 5: Attempt to register with an empty name
        System.out.println("5. " + registerUser(" ", "david@example.com", "ValidPassword101"));

        // Print database contents (for verification)
        System.out.println("\nCurrent users in database:");
        userDatabase.forEach((email, user) -> System.out.println("- " + user.toString()));
    }
}