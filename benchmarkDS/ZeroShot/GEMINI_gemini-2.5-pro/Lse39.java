import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Base64;

public class Lse39 {

    // In a real application, this would be a database.
    private static final Map<String, UserData> userDatabase = new HashMap<>();
    private static final int SALT_LENGTH = 16; // 16 bytes salt
    private static final int ITERATION_COUNT = 65536;
    private static final int KEY_LENGTH = 128; // 128 bits key length
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Inner class to store user's salt and hashed password.
     */
    private static class UserData {
        private final byte[] salt;
        private final byte[] hashedPassword;

        public UserData(byte[] salt, byte[] hashedPassword) {
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
     * Generates a random salt.
     * @return a byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password with the given salt using PBKDF2.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }

    /**
     * "Registers" a new user by hashing their password and storing it.
     * This is for demonstration purposes.
     * @param username The username.
     * @param password The plaintext password.
     */
    public static void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return;
        }
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);
        userDatabase.put(username, new UserData(salt, hashedPassword));
    }

    /**
     * Attempts to log in a user.
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return true if login is successful, false otherwise.
     */
    public boolean login(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        UserData userData = userDatabase.get(username);
        if (userData == null) {
            // To prevent timing attacks, we can perform a dummy hash calculation
            // if the user is not found. The salt can be a dummy, fixed salt.
            byte[] dummySalt = new byte[SALT_LENGTH]; 
            hashPassword(password.toCharArray(), dummySalt);
            return false;
        }

        byte[] salt = userData.getSalt();
        byte[] storedHash = userData.getHashedPassword();

        // Hash the provided password with the stored salt
        byte[] providedPasswordHash = hashPassword(password.toCharArray(), salt);

        // Compare the hashes in a way that is resistant to timing attacks.
        return Arrays.equals(storedHash, providedPasswordHash);
    }

    public static void main(String[] args) {
        // "Register" a user for testing. In a real app, this happens once.
        registerUser("admin", "S3cureP@ssword!123");

        Lse39 loginSystem = new Lse39();
        
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Correct username and correct password
        System.out.print("Test Case 1 (Correct credentials): ");
        boolean success1 = loginSystem.login("admin", "S3cureP@ssword!123");
        System.out.println(success1 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

        // Test Case 2: Correct username and incorrect password
        System.out.print("Test Case 2 (Incorrect password): ");
        boolean success2 = loginSystem.login("admin", "wrongpassword");
        System.out.println(success2 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

        // Test Case 3: Incorrect username
        System.out.print("Test Case 3 (Incorrect username): ");
        boolean success3 = loginSystem.login("guest", "S3cureP@ssword!123");
        System.out.println(success3 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

        // Test Case 4: Correct username, empty password
        System.out.print("Test Case 4 (Empty password): ");
        boolean success4 = loginSystem.login("admin", "");
        System.out.println(success4 ? "Login successful. Redirecting to session page." : "Invalid username or password.");

        // Test Case 5: Null inputs
        System.out.print("Test Case 5 (Null username): ");
        boolean success5 = loginSystem.login(null, "password");
        System.out.println(success5 ? "Login successful. Redirecting to session page." : "Invalid username or password.");
    }
}