import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.HexFormat;

public class Task82 {

    private static final int ITERATIONS = 600000;
    private static final int KEY_LENGTH = 256; // bits
    private static final int SALT_LENGTH = 16; // bytes
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return A 16-byte salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Computes the hash of a password combined with a salt using PBKDF2 with HMAC-SHA256.
     *
     * @param password The password to hash.
     * @param salt     The salt to use.
     * @return The resulting hash as a hex-encoded string.
     * @throws NoSuchAlgorithmException if the PBKDF2WithHmacSHA256 algorithm is not available.
     * @throws InvalidKeySpecException  if the provided key specification is inappropriate.
     */
    public static String hashPassword(char[] password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        if (password == null || salt == null) {
            throw new IllegalArgumentException("Password and salt must not be null.");
        }
        
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            byte[] hash = skf.generateSecret(spec).getEncoded();
            return HexFormat.of().formatHex(hash);
        } finally {
            spec.clearPassword();
        }
    }

    public static void main(String[] args) {
        String[] testPasswords = {
            "P@ssword123!",
            "correct horse battery staple",
            "Tr0ub4dor&3",
            "123456",
            "super-secret-password"
        };

        System.out.println("Running 5 test cases for password hashing...");
        for (String passwordStr : testPasswords) {
            char[] passwordChars = passwordStr.toCharArray();
            try {
                // In a real application, never log or print the plaintext password.
                // This is for demonstration purposes only.
                System.out.println("\n--- Test Case ---");
                System.out.println("Password (for demo only): " + passwordStr);

                byte[] salt = generateSalt();
                System.out.println("Generated Salt (hex): " + HexFormat.of().formatHex(salt));

                String hashedPassword = hashPassword(passwordChars, salt);
                
                System.out.println("Hashed Password (hex): " + hashedPassword);

            } catch (NoSuchAlgorithmException | InvalidKeySpecException | IllegalArgumentException e) {
                System.err.println("An error occurred during hashing: " + e.getMessage());
                e.printStackTrace();
            } finally {
                // Clear the password from memory as soon as it's not needed.
                Arrays.fill(passwordChars, '\0');
            }
        }
    }
}