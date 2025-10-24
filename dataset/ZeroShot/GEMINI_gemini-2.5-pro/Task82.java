import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Formatter;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task82 {

    private static final int ITERATIONS = 65536;
    private static final int KEY_LENGTH = 256; // bits
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Computes the hash of a password combined with a salt using PBKDF2 with HMAC-SHA256.
     *
     * @param password The password to hash.
     * @param salt     The salt to use for hashing. Must be a cryptographically random sequence of bytes.
     * @return The resulting hash as a byte array.
     */
    public static byte[] hashPassword(final char[] password, final byte[] salt) {
        if (password == null || salt == null) {
            throw new IllegalArgumentException("Password and salt must not be null.");
        }
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            // It's good practice to clear the password from memory after use
            Arrays.fill(password, Character.MIN_VALUE);
            SecretKeyFactory skf = SecretKeyFactory.getInstance(ALGORITHM);
            return skf.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // These exceptions should not happen with a standard algorithm
            throw new RuntimeException("Error while hashing a password: " + e.getMessage(), e);
        }
    }

    /**
     * Helper method to convert a byte array to a hexadecimal string.
     */
    private static String toHexString(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        try (Formatter formatter = new Formatter(sb)) {
            for (byte b : bytes) {
                formatter.format("%02x", b);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] passwords = {
            "password123",
            "P@$$w0rd!",
            "correct horse battery staple",
            "123",
            ""
        };

        byte[][] salts = {
            "salt1234".getBytes(StandardCharsets.UTF_8),
            new byte[]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
            "another-random-salt".getBytes(StandardCharsets.UTF_8),
            new byte[0], // Empty salt
            "a_very_long_salt_for_testing_purposes_with_an_empty_password".getBytes(StandardCharsets.UTF_8)
        };

        System.out.println("Java Password Hashing Tests:");
        for (int i = 0; i < passwords.length; i++) {
            String password = passwords[i];
            byte[] salt = salts[i];
            
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Password: \"" + password + "\"");
            System.out.println("Salt (hex): " + toHexString(salt));
            
            // Use a copy for hashing to keep the original for printing
            byte[] hash = hashPassword(password.toCharArray(), salt);
            System.out.println("Hashed Password (hex): " + toHexString(hash));
        }
    }
}