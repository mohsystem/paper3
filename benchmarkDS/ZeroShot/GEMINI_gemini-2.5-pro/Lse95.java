import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse95 {

    // In-memory map to simulate a database
    private static final Map<String, UserCredentials> userDatabase = new HashMap<>();
    private static final int SALT_LENGTH = 16; // 16 bytes = 128 bits
    private static final int ITERATION_COUNT = 65536;
    private static final int KEY_LENGTH = 128; // in bits

    // Class to hold salt and hashed password
    private static class UserCredentials {
        final byte[] salt;
        final byte[] hashedPassword;

        UserCredentials(byte[] salt, byte[] hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }

    /**
     * Generates a random salt.
     * @return A byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with a given salt.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * @param username The username.
     * @param password The plaintext password.
     * @return true if registration is successful, false if user already exists.
     */
    public static boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Username and password cannot be empty.");
            return false;
        }
        if (userDatabase.containsKey(username)) {
            return false; // User already exists
        }

        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);

        userDatabase.put(username, new UserCredentials(salt, hashedPassword));
        return true;
    }

    /**
     * Verifies a user's password during login.
     * @param username The username.
     * @param password The plaintext password to check.
     * @return true if the password is correct, false otherwise.
     */
    public static boolean verifyPassword(String username, String password) {
        UserCredentials credentials = userDatabase.get(username);
        if (credentials == null) {
            return false; // User not found
        }

        byte[] providedPasswordHash = hashPassword(password.toCharArray(), credentials.salt);
        return Arrays.equals(providedPasswordHash, credentials.hashedPassword);
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Successful registration
        boolean reg1 = registerUser("alice", "Password123!");
        System.out.println("Test 1 - Alice registration: " + (reg1 ? "Success" : "Fail"));

        // Test Case 2: Successful login
        boolean login1 = verifyPassword("alice", "Password123!");
        System.out.println("Test 2 - Alice successful login: " + (login1 ? "Success" : "Fail"));

        // Test Case 3: Failed login (wrong password)
        boolean login2 = verifyPassword("alice", "WrongPassword");
        System.out.println("Test 3 - Alice failed login (wrong password): " + (!login2 ? "Success" : "Fail"));

        // Test Case 4: Failed login (non-existent user)
        boolean login3 = verifyPassword("bob", "Password123!");
        System.out.println("Test 4 - Bob failed login (user not found): " + (!login3 ? "Success" : "Fail"));

        // Test Case 5: Failed registration (duplicate username)
        boolean reg2 = registerUser("alice", "AnotherPassword");
        System.out.println("Test 5 - Alice duplicate registration: " + (!reg2 ? "Success" : "Fail"));

        // Optional: Print stored data for verification
        UserCredentials creds = userDatabase.get("alice");
        if (creds != null) {
            System.out.println("\nStored data for Alice:");
            System.out.println("Salt: " + Base64.getEncoder().encodeToString(creds.salt));
            System.out.println("Hashed Password: " + Base64.getEncoder().encodeToString(creds.hashedPassword));
        }
    }
}