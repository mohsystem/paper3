import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import java.security.MessageDigest;

/**
 * Main class for the login simulation.
 * The class name must be Lse35 as per instructions.
 */
public class Lse35 {

    // Simulate a user database using a Map
    private static final Map<String, User> userDatabase = new HashMap<>();

    // Cryptographic constants for password hashing
    private static final int SALT_LENGTH = 16; // 16 bytes for salt
    private static final int HASH_LENGTH = 256; // a 256-bit hash
    private static final int ITERATIONS = 100000; // Iteration count for PBKDF2
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    // Rule #1: Use regex for input validation
    // Username: 3-20 chars, alphanumeric
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9]{3,20}$");
    // Password: 8-64 chars, at least one letter and one digit
    private static final Pattern PASSWORD_PATTERN = Pattern.compile("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,64}$");


    /**
     * Inner class to represent a user's secure credentials.
     */
    private static class User {
        String username;
        byte[] salt;
        byte[] hashedPassword;

        User(String username, byte[] salt, byte[] hashedPassword) {
            this.username = username;
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }

    /**
     * Generates a cryptographically secure random salt.
     * Rule #10: Use a strong, cryptographically secure random number generator.
     * @return A random salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password with a given salt using PBKDF2.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_LENGTH);
            // Rule #15: Defensively clear the password from memory after use
            Arrays.fill(password, Character.MIN_VALUE);
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return skf.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this securely.
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * Registers a new user in the simulated database.
     * This function is for setting up the simulation.
     * @param username The username.
     * @param password The password.
     */
    public static void registerUser(String username, String password) {
        if (username == null || password == null) {
            System.err.println("Registration failed: Username or password cannot be null.");
            return;
        }
        // Rule #9: Use a unique salt for each password
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);
        userDatabase.put(username, new User(username, salt, hashedPassword));
        System.out.println("User '" + username + "' registered.");
    }

    /**
     * Simulates a login function that would be called by a web server.
     * It checks credentials and determines redirection.
     *
     * @param username The username from the form.
     * @param password The password from the form.
     * @return "HOME_PAGE" on successful authentication, "LOGIN_PAGE" on failure.
     */
    public static String login(String username, String password) {
        // Rule #1: Ensure all input is validated and sanitized
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Error: Invalid username format.");
            return "LOGIN_PAGE";
        }
        if (password == null || !PASSWORD_PATTERN.matcher(password).matches()) {
            System.err.println("Error: Invalid password format.");
            return "LOGIN_PAGE";
        }

        User user = userDatabase.get(username);
        if (user == null) {
            // To prevent username enumeration via timing attacks, perform a dummy hash
            // with a random salt. This takes roughly the same time as a real check.
            byte[] dummySalt = generateSalt();
            hashPassword("dummyPassword".toCharArray(), dummySalt);
            return "LOGIN_PAGE";
        }

        byte[] providedPasswordHash = hashPassword(password.toCharArray(), user.salt);

        // Rule #7 & #8: Use a vetted crypto library and constant-time comparison for secrets
        if (MessageDigest.isEqual(providedPasswordHash, user.hashedPassword)) {
            return "HOME_PAGE";
        } else {
            return "LOGIN_PAGE";
        }
    }

    /**
     * Main method with 5 test cases to demonstrate the login functionality.
     */
    public static void main(String[] args) {
        // Populate the mock database with some users
        registerUser("admin", "AdminPass123");
        registerUser("user1", "UserPass456");

        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Successful login
        System.out.println("\nTest Case 1: Valid credentials for 'admin'");
        String result1 = login("admin", "AdminPass123");
        System.out.println("Result: Login " + (result1.equals("HOME_PAGE") ? "successful" : "failed") + ". Redirecting to " + result1);

        // Test Case 2: Incorrect password
        System.out.println("\nTest Case 2: Invalid password for 'admin'");
        String result2 = login("admin", "WrongPass789");
        System.out.println("Result: Login " + (result2.equals("HOME_PAGE") ? "successful" : "failed") + ". Redirecting to " + result2);

        // Test Case 3: Non-existent user
        System.out.println("\nTest Case 3: Non-existent user 'nouser'");
        String result3 = login("nouser", "SomePass123");
        System.out.println("Result: Login " + (result3.equals("HOME_PAGE") ? "successful" : "failed") + ". Redirecting to " + result3);
        
        // Test Case 4: Invalid username format
        System.out.println("\nTest Case 4: Invalid username format 'us'");
        String result4 = login("us", "SomePass123");
        System.out.println("Result: Login " + (result4.equals("HOME_PAGE") ? "successful" : "failed") + ". Redirecting to " + result4);

        // Test Case 5: Invalid password format (too short)
        System.out.println("\nTest Case 5: Invalid password format for 'user1'");
        String result5 = login("user1", "pass");
        System.out.println("Result: Login " + (result5.equals("HOME_PAGE") ? "successful" : "failed") + ". Redirecting to " + result5);
    }
}