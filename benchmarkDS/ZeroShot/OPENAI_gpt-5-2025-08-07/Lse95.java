import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.util.Base64;

public class Lse95 {
    private static final Map<String, String> DB = new ConcurrentHashMap<>();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 100_000;
    private static final int KEY_LEN = 32; // 256-bit
    private static final SecureRandom RNG = createSecureRandom();

    private static SecureRandom createSecureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (Exception e) {
            return new SecureRandom();
        }
    }

    public static boolean registerUser(String username, char[] password) {
        try {
            if (!isValidUsername(username) || !isValidPassword(password)) {
                return false;
            }
            if (DB.containsKey(username)) {
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] dk = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
            String stored = toHex(dk) + ":" + toHex(salt);
            DB.put(username, stored);
            return true;
        } catch (Exception e) {
            return false;
        } finally {
            zeroize(password);
        }
    }

    public static boolean loginUser(String username, char[] password) {
        try {
            String stored = DB.get(username);
            if (stored == null) return false;
            String[] parts = stored.split(":");
            if (parts.length != 2) return false;
            byte[] storedHash = fromHex(parts[0]);
            byte[] salt = fromHex(parts[1]);
            byte[] dk = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
            return MessageDigest.isEqual(storedHash, dk);
        } catch (Exception e) {
            return false;
        } finally {
            zeroize(password);
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isValidPassword(char[] password) {
        if (password == null || password.length < 8) return false;
        return true;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    private static byte[] fromHex(String hex) {
        int len = hex.length();
        if ((len & 1) != 0) throw new IllegalArgumentException("Invalid hex");
        byte[] out = new byte[len / 2];
        for (int i = 0; i < out.length; i++) {
            int j = i * 2;
            out[i] = (byte) Integer.parseInt(hex.substring(j, j + 2), 16);
        }
        return out;
    }

    private static void zeroize(char[] data) {
        if (data != null) Arrays.fill(data, '\0');
    }

    // Test cases
    public static void main(String[] args) {
        System.out.println("Register alice: " + registerUser("alice", "Str0ngP@ss!".toCharArray()));
        System.out.println("Login alice (correct): " + loginUser("alice", "Str0ngP@ss!".toCharArray()));
        System.out.println("Register bob: " + registerUser("bob", "anotherS3cure".toCharArray()));
        System.out.println("Login bob (wrong): " + loginUser("bob", "wrongpass".toCharArray()));
        System.out.println("Register alice again (duplicate): " + registerUser("alice", "NewPassw0rd!".toCharArray()));
        System.out.println("Register charlie_01 (too short): " + registerUser("charlie_01", "short".toCharArray()));
        System.out.println("Register dave: " + registerUser("dave", "CorrectHorseBatteryStaple1!".toCharArray()));
        System.out.println("Login dave (correct): " + loginUser("dave", "CorrectHorseBatteryStaple1!".toCharArray()));
        System.out.println("Login unknown user: " + loginUser("eve", "SomePass123".toCharArray()));
    }
}