import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Lse38 {

    // In-memory user store to simulate a database.
    // In a real application, this would be a database table.
    private static final Map<String, UserData> userDatabase = new HashMap<>();
    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 256; // bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Represents user data stored in the database.
     * Includes a unique salt for each user to prevent rainbow table attacks.
     */
    private static class UserData {
        private final String username;
        private final byte[] salt;
        private final byte[] hashedPassword;

        UserData(String username, byte[] salt, byte[] hashedPassword) {
            this.username = username;
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }

        public byte[] getSalt() {
            return salt;
        }

        public byte[] getHashedPassword() {
            return hashedPassword;
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
     * Hashes the password using PBKDF2 with a given salt.
     * @return Hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        Arrays.fill(password, Character.MIN_VALUE); // Clear password from memory
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
     * Simulates adding a new user to the database.
     * In a real app, this would be a user registration process.
     */
    private static void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return;
        }
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);
        UserData newUser = new UserData(username, salt, hashedPassword);
        userDatabase.put(username.toLowerCase(), newUser); // Store username in lowercase for case-insensitivity
    }
    
    /**
     * Securely verifies a user's password.
     *
     * @param providedPassword The password entered by the user.
     * @param user             The user data retrieved from the database.
     * @return True if the password is correct, false otherwise.
     */
    private static boolean verifyPassword(String providedPassword, UserData user) {
        if (providedPassword == null || user == null) {
            return false;
        }
        // Hash the provided password with the user's stored salt
        byte[] providedHash = hashPassword(providedPassword.toCharArray(), user.getSalt());
        
        // Compare the hashes in a way that resists timing attacks.
        return Arrays.equals(providedHash, user.getHashedPassword());
    }


    /**
     * Simulates a login attempt.
     *
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return A string indicating the result of the login attempt.
     */
    public static String login(String username, String password) {
        // Basic input validation
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            return "Login failed: Username or password cannot be empty. Redirecting to login page.";
        }

        // Retrieve user data from the database (case-insensitive username)
        Optional<UserData> userOpt = Optional.ofNullable(userDatabase.get(username.toLowerCase()));

        if (userOpt.isPresent()) {
            UserData user = userOpt.get();
            // Securely verify the password
            if (verifyPassword(password, user)) {
                return "Login successful for " + username + "! Redirecting to dashboard.";
            }
        }
        
        // Generic failure message to prevent username enumeration
        return "Login failed: Invalid username or password. Redirecting to login page.";
    }

    public static void main(String[] args) {
        // --- Setup: Simulate user registration ---
        registerUser("admin", "AdminPassword123!");
        registerUser("jdoe", "P@ssw0rd_jdoe");
        System.out.println("--- User database initialized ---");

        // --- Test Cases ---
        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Correct username and password
        System.out.println("1. " + login("admin", "AdminPassword123!"));

        // Test Case 2: Correct username, incorrect password
        System.out.println("2. " + login("admin", "WrongPassword"));
        
        // Test Case 3: Incorrect username
        System.out.println("3. " + login("unknownUser", "anyPassword"));

        // Test Case 4: Correct username (different case), correct password
        System.out.println("4. " + login("Admin", "AdminPassword123!"));

        // Test Case 5: Empty password
        System.out.println("5. " + login("jdoe", ""));
        
        // Test Case 6: Empty username
        System.out.println("6. " + login("", "P@ssw0rd_jdoe"));
        
        // Test Case 7: Null username
        System.out.println("7. " + login(null, "P@ssw0rd_jdoe"));
    }
}