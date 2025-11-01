import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse135 {

    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 32; // 256-bit
    private static final int ITERATIONS = 210000;

    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    private static final Map<String, UserRecord> STORE = new HashMap<>();

    private static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    // Returns "true" or "false"
    public static String check_mod(String username, String password) {
        if (!isValidUsername(username) || !isValidPasswordForLogin(password)) {
            return "false";
        }

        UserRecord rec = STORE.get(username);
        if (rec == null) {
            return "false";
        }

        char[] pwChars = password.toCharArray();
        byte[] derived = null;
        boolean match = false;
        try {
            derived = pbkdf2(pwChars, rec.salt, rec.iterations, KEY_LEN);
            match = constantTimeEquals(derived, rec.hash);
        } catch (Exception e) {
            // Fail closed
            match = false;
        } finally {
            if (derived != null) {
                Arrays.fill(derived, (byte) 0);
            }
            Arrays.fill(pwChars, '\0');
        }
        return match ? "true" : "false";
    }

    private static boolean isValidUsername(String u) {
        if (u == null) return false;
        String trimmed = u.trim();
        if (trimmed.length() == 0 || trimmed.length() > 64) return false;
        // Allow alphanumerics and limited safe symbols
        for (int i = 0; i < trimmed.length(); i++) {
            char c = trimmed.charAt(i);
            if (!(c >= 'a' && c <= 'z') &&
                !(c >= 'A' && c <= 'Z') &&
                !(c >= '0' && c <= '9') &&
                c != '_' && c != '-' && c != '.') {
                return false;
            }
        }
        return true;
    }

    private static boolean isValidPasswordForLogin(String p) {
        if (p == null) return false;
        int len = p.length();
        return len >= 1 && len <= 1024;
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= (a[i] ^ b[i]);
        }
        return result == 0;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static void registerUser(String username, char[] password) {
        if (!isValidUsername(username)) {
            throw new IllegalArgumentException("Invalid username");
        }
        if (!isPasswordStrong(password)) {
            Arrays.fill(password, '\0');
            throw new IllegalArgumentException("Password does not meet policy");
        }
        byte[] salt = new byte[SALT_LEN];
        SECURE_RANDOM.nextBytes(salt);
        byte[] hash = null;
        try {
            hash = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
            STORE.put(username, new UserRecord(salt, hash, ITERATIONS));
        } catch (Exception e) {
            Arrays.fill(salt, (byte) 0);
            if (hash != null) Arrays.fill(hash, (byte) 0);
            throw new RuntimeException("Registration failed");
        } finally {
            Arrays.fill(password, '\0');
        }
    }

    // Basic strong password policy: length >= 12, includes upper, lower, digit, and symbol
    private static boolean isPasswordStrong(char[] pw) {
        if (pw == null || pw.length < 12 || pw.length > 1024) return false;
        boolean hasU = false, hasL = false, hasD = false, hasS = false;
        for (char c : pw) {
            if (c >= 'A' && c <= 'Z') hasU = true;
            else if (c >= 'a' && c <= 'z') hasL = true;
            else if (c >= '0' && c <= '9') hasD = true;
            else hasS = true;
        }
        return hasU && hasL && hasD && hasS;
    }

    public static void main(String[] args) {
        // Initialize a few users securely (no hardcoded plaintext storage)
        registerUser("alice", "Str0ng!Pass#1".toCharArray());
        registerUser("bob", "Another$Pass2".toCharArray());
        registerUser("eve", "Complex%Pass3".toCharArray());

        // 5 test cases
        System.out.println(check_mod("alice", "Str0ng!Pass#1")); // true
        System.out.println(check_mod("alice", "wrong"));         // false
        System.out.println(check_mod("unknown", "whatever"));    // false
        System.out.println(check_mod("", "nope"));               // false
        System.out.println(check_mod("bob", "Another$Pass2"));   // true
    }
}