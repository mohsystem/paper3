import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;

public class Lse96 {

    private static final int SALT_LENGTH = 16; // 16 bytes salt
    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 256; // 256 bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return A byte array containing the salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 algorithm.
     *
     * @param password The password to hash.
     * @param salt     The salt to use for hashing.
     * @return The hashed password as a byte array.
     */
    public static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return skf.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new AssertionError("Error while hashing a password: " + e.getMessage(), e);
        }
    }

    /**
     * Simulates user registration by generating a salt, hashing the password,
     * and concatenating them for storage.
     *
     * @param username The username (not used in hashing but part of a typical registration).
     * @param password The plain-text password.
     * @return A string representing the salt and hashed password, ready for storage.
     */
    public static String registerUser(String username, String password) {
        if (password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Password cannot be null or empty.");
        }

        // 1. Generate a salt
        byte[] salt = generateSalt();

        // 2. Hash the password with the salt
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);

        // 3. Encode salt and hash to Base64 for safe string storage
        String encodedSalt = Base64.getEncoder().encodeToString(salt);
        String encodedHash = Base64.getEncoder().encodeToString(hashedPassword);

        // 4. Concatenate salt and hash for storage (a separator is good practice)
        return encodedSalt + ":" + encodedHash;
    }


    public static void main(String[] args) {
        System.out.println("--- Running Java Registration Test Cases ---");

        // Test Case 1
        String storedCredentials1 = registerUser("user1", "Password123!");
        System.out.println("User: user1, Stored: " + storedCredentials1);

        // Test Case 2
        String storedCredentials2 = registerUser("admin_test", "S3cureP@ssw0rd");
        System.out.println("User: admin_test, Stored: " + storedCredentials2);

        // Test Case 3
        String storedCredentials3 = registerUser("jane.doe", "another-password-4-jane");
        System.out.println("User: jane.doe, Stored: " + storedCredentials3);

        // Test Case 4: Simple password
        String storedCredentials4 = registerUser("test_user", "password");
        System.out.println("User: test_user, Stored: " + storedCredentials4);

        // Test Case 5: Long password with many special characters
        String storedCredentials5 = registerUser("poweruser", "L0ngP@$$w0rd_With_Many_Ch@r$!#%^&*()");
        System.out.println("User: poweruser, Stored: " + storedCredentials5);
    }
}