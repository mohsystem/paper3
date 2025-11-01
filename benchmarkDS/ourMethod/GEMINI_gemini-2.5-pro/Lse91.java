import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse91 {

    // In-memory database simulation
    private static final Map<String, String> userDatabase = new HashMap<>();

    // Constants for password hashing
    private static final int SALT_LENGTH_BYTES = 16;
    private static final int HASH_LENGTH_BITS = 256;
    // Iteration count should be high. OWASP recommends at least 310,000 for PBKDF2-SHA256.
    private static final int ITERATIONS = 310000;
    private static final String HASHING_ALGORITHM = "PBKDF2WithHmacSHA256";

    // Regex for basic input validation
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,20}$");
    // Password policy: 8-100 chars, at least one uppercase, one lowercase, one digit, one special char.
    private static final Pattern PASSWORD_PATTERN = Pattern.compile("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,100}$");


    /**
     * Hashes a password using PBKDF2 with a random salt.
     *
     * @param password The password to hash.
     * @return A formatted string containing the algorithm, iterations, salt, and hash.
     * @throws NoSuchAlgorithmException if the hashing algorithm is not available.
     * @throws InvalidKeySpecException if the key spec is invalid.
     */
    private static String hashPassword(char[] password) throws NoSuchAlgorithmException, InvalidKeySpecException {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH_BYTES];
        random.nextBytes(salt);

        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_LENGTH_BITS);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASHING_ALGORITHM);
            byte[] hash = skf.generateSecret(spec).getEncoded();
            
            // Encode salt and hash to Base64 for safe string storage
            String encodedSalt = Base64.getEncoder().encodeToString(salt);
            String encodedHash = Base64.getEncoder().encodeToString(hash);

            return HASHING_ALGORITHM + ":" + ITERATIONS + ":" + encodedSalt + ":" + encodedHash;
        } finally {
            // Clear the password from memory
            spec.clearPassword();
        }
    }

    /**
     * Registers a new user after validating inputs and hashing the password.
     *
     * @param username The username for the new account.
     * @param password The plaintext password for the new account.
     * @return true if registration is successful, false otherwise.
     */
    public static boolean registerUser(String username, String password) {
        // Rule #1: Input Validation
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Registration failed: Invalid username format.");
            return false;
        }
        if (password == null || !PASSWORD_PATTERN.matcher(password).matches()) {
            System.err.println("Registration failed: Password does not meet policy requirements.");
            return false;
        }
        if (userDatabase.containsKey(username)) {
            System.err.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }

        char[] passwordChars = password.toCharArray();
        try {
            String hashedPassword = hashPassword(passwordChars);
            userDatabase.put(username, hashedPassword);
            System.out.println("User '" + username + "' registered successfully.");
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Critical error during password hashing: " + e.getMessage());
            return false;
        } finally {
            // Clear the plaintext password from memory
            Arrays.fill(passwordChars, '\0');
        }
    }

    public static void main(String[] args) {
        System.out.println("--- User Registration Simulation ---");

        // Test Case 1: Successful registration
        registerUser("alice_123", "ValidPass1!");

        // Test Case 2: Successful registration with another user
        registerUser("bob_smith", "Another_Good_P@ssw0rd");

        // Test Case 3: Failed registration - duplicate username
        registerUser("alice_123", "SomeOtherPass1?");

        // Test Case 4: Failed registration - invalid username
        registerUser("!!invalid-user!!", "ValidPass1!");

        // Test Case 5: Failed registration - weak password
        registerUser("charlie", "weak");

        // Display the contents of the simulated database
        System.out.println("\n--- Simulated User Database ---");
        userDatabase.forEach((username, hash) -> System.out.println("Username: " + username + "\nStored Hash: " + hash + "\n"));
    }
}