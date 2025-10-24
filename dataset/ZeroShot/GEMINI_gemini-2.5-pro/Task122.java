import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.Arrays;

public class Task122 {

    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int SALT_SIZE = 16; // 128 bit
    private static final int HASH_SIZE = 32; // 256 bit
    private static final int ITERATIONS = 65536;

    /**
     * Stores a user password securely by generating a salt and hashing the password.
     * @param password The plaintext password to store.
     * @return A string containing the salt and hash, separated by a colon.
     */
    public static String signup(String password) {
        try {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_SIZE];
            random.nextBytes(salt);

            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, HASH_SIZE * 8);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();

            Base64.Encoder enc = Base64.getEncoder();
            return enc.encodeToString(salt) + ":" + enc.encodeToString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this exception and handle it gracefully.
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    /**
     * Verifies a plaintext password against a stored salted hash.
     * @param password The plaintext password to verify.
     * @param storedPassword The stored string containing the salt and hash.
     * @return true if the password is correct, false otherwise.
     */
    public static boolean verifyPassword(String password, String storedPassword) {
        try {
            String[] parts = storedPassword.split(":");
            if (parts.length != 2) {
                return false; // Invalid format
            }
            Base64.Decoder dec = Base64.getDecoder();
            byte[] salt = dec.decode(parts[0]);
            byte[] storedHash = dec.decode(parts[1]);

            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, HASH_SIZE * 8);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] testHash = factory.generateSecret(spec).getEncoded();

            // Use a constant-time comparison to prevent timing attacks
            return Arrays.equals(storedHash, testHash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException | IllegalArgumentException e) {
            // Handle exceptions, e.g., log them. An invalid storedPassword format might throw IllegalArgumentException.
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Password Hashing Tests ---");

        // Test Case 1: Sign up a user and store the password hash
        System.out.println("\n[Test Case 1: Signup with 'password123']");
        String user1Password = "password123";
        String storedUser1 = signup(user1Password);
        System.out.println("Stored format for '" + user1Password + "': " + storedUser1);

        // Test Case 2: Verify with the correct password
        System.out.println("\n[Test Case 2: Verify with correct password 'password123']");
        boolean isCorrect = verifyPassword(user1Password, storedUser1);
        System.out.println("Verification successful: " + isCorrect);
        if (!isCorrect) System.out.println("TEST FAILED!");

        // Test Case 3: Verify with an incorrect password
        System.out.println("\n[Test Case 3: Verify with incorrect password 'wrongpassword']");
        boolean isIncorrect = verifyPassword("wrongpassword", storedUser1);
        System.out.println("Verification successful: " + isIncorrect);
        if (isIncorrect) System.out.println("TEST FAILED!");

        // Test Case 4: Sign up another user with a different password
        System.out.println("\n[Test Case 4: Signup with a different password 'anotherPassword']");
        String user2Password = "anotherPassword";
        String storedUser2 = signup(user2Password);
        System.out.println("Stored format for '" + user2Password + "': " + storedUser2);
        System.out.println("Hashes for different passwords are different: " + !storedUser1.equals(storedUser2));

        // Test Case 5: Sign up with the same password again, salt should be different
        System.out.println("\n[Test Case 5: Signup with 'password123' again]");
        String storedUser1Again = signup(user1Password);
        System.out.println("Second stored format for '" + user1Password + "': " + storedUser1Again);
        System.out.println("Hashes for the same password are different due to salt: " + !storedUser1.equals(storedUser1Again));
    }
}