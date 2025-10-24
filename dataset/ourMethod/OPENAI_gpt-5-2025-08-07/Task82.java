import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task82 {
    private static final int SALT_LENGTH = 16; // 16 bytes
    private static final int ITERATIONS = 210_000;
    private static final int DK_LEN = 32; // 32 bytes = 256 bits

    public static byte[] generateSalt(int length) {
        if (length <= 0 || length > 1024) {
            throw new IllegalArgumentException("Invalid salt length.");
        }
        byte[] salt = new byte[length];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(salt);
        return salt;
    }

    public static String hashPassword(char[] password, byte[] salt, int iterations, int dkLenBytes) {
        if (password == null || password.length == 0) {
            throw new IllegalArgumentException("Password must not be empty.");
        }
        if (!isPasswordStrong(password)) {
            throw new IllegalArgumentException("Password does not meet strength policy.");
        }
        if (salt == null || salt.length < 8 || salt.length > 1024) {
            throw new IllegalArgumentException("Invalid salt.");
        }
        if (iterations < 100_000 || iterations > 5_000_000) {
            throw new IllegalArgumentException("Invalid iterations.");
        }
        if (dkLenBytes < 16 || dkLenBytes > 64) {
            throw new IllegalArgumentException("Invalid derived key length.");
        }

        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLenBytes * 8);
        byte[] key = null;
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            key = skf.generateSecret(spec).getEncoded();
            return toHex(key);
        } catch (Exception e) {
            throw new RuntimeException("Hashing failed.", e);
        } finally {
            spec.clearPassword();
            if (key != null) Arrays.fill(key, (byte) 0);
        }
    }

    private static boolean isPasswordStrong(char[] pwd) {
        if (pwd.length < 12) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : pwd) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    // 5 test cases
    public static void main(String[] args) {
        String[] passwords = new String[] {
            "Str0ngPassw0rd!",
            "An0ther$ecurePwd",
            "C0mpl3x#Pass123",
            "S@feAndS0und2025",
            "R0bust_P@ssw0rd!!"
        };
        for (int i = 0; i < passwords.length; i++) {
            char[] pw = passwords[i].toCharArray();
            byte[] salt = generateSalt(SALT_LENGTH);
            String hash = hashPassword(pw, salt, ITERATIONS, DK_LEN);
            Arrays.fill(pw, '\0');
            System.out.println("Test " + (i + 1) + " hash: " + hash);
        }
    }
}