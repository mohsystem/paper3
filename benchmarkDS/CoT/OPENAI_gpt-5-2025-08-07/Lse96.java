import java.security.SecureRandom;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.*;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse96 {
    private static final Map<String, String> DATABASE = new HashMap<>();
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 100_000;
    private static final int KEY_LEN_BITS = 256;

    public static String registerUser(String username, char[] password) {
        if (username == null || username.isBlank() || password == null || password.length == 0) {
            zeroize(password);
            return null;
        }
        synchronized (DATABASE) {
            if (DATABASE.containsKey(username)) {
                zeroize(password);
                return null;
            }
            byte[] salt = generateSalt(SALT_LEN);
            byte[] hash = hashPassword(password, salt, ITERATIONS, KEY_LEN_BITS);
            String stored = toHex(salt) + ":" + toHex(hash);
            DATABASE.put(username, stored);
            zeroize(password);
            Arrays.fill(salt, (byte) 0);
            Arrays.fill(hash, (byte) 0);
            return stored;
        }
    }

    private static byte[] generateSalt(int length) {
        byte[] salt = new byte[length];
        new SecureRandom().nextBytes(salt);
        return salt;
    }

    private static byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLengthBits) {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLengthBits);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error hashing password", e);
        } finally {
            spec.clearPassword();
        }
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    private static void zeroize(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Register alice: " + registerUser("alice", "Password1!".toCharArray()));
        System.out.println("Register bob: " + registerUser("bob", "S3cure#Pass".toCharArray()));
        System.out.println("Register carol: " + registerUser("carol", "anotherPass123".toCharArray()));
        System.out.println("Register alice again (should fail): " + registerUser("alice", "newPass!".toCharArray()));
        System.out.println("Register dave: " + registerUser("dave", "p@ssW0rd".toCharArray()));
    }
}