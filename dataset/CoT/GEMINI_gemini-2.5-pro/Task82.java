import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.math.BigInteger;

public class Task82 {

    private static final int ITERATIONS = 100000;
    private static final int KEY_LENGTH = 256; // in bits
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Computes the hash of a password combined with a salt using PBKDF2.
     *
     * @param password The password to hash.
     * @param salt     The salt to use.
     * @return The hex-encoded hash string, or null on failure.
     */
    public static String computeHash(String password, String salt) {
        try {
            byte[] saltBytes = salt.getBytes(StandardCharsets.UTF_8);
            KeySpec spec = new PBEKeySpec(password.toCharArray(), saltBytes, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return toHexString(hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, this should be logged.
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Converts a byte array into a hexadecimal string.
     *
     * @param bytes The byte array to convert.
     * @return The hexadecimal string representation.
     */
    private static String toHexString(byte[] bytes) {
        BigInteger bi = new BigInteger(1, bytes);
        String hex = bi.toString(16);
        int paddingLength = (bytes.length * 2) - hex.length();
        if (paddingLength > 0) {
            return String.format("%0" + paddingLength + "d", 0) + hex;
        } else {
            return hex;
        }
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {"password123", "somesalt1"},
            {"P@$$w0rd!", "anothersalt2"},
            {"a_very_secure_password", "salty-salt-salt"},
            {"", "emptypassword"}, // Edge case: empty password
            {"test", ""}          // Edge case: empty salt
        };

        System.out.println("Java PBKDF2-HMAC-SHA256 Hashes:");
        for (String[] testCase : testCases) {
            String password = testCase[0];
            String salt = testCase[1];
            String hash = computeHash(password, salt);
            System.out.println("Password: \"" + password + "\", Salt: \"" + salt + "\"");
            System.out.println("Hash: " + hash);
            System.out.println();
        }
    }
}