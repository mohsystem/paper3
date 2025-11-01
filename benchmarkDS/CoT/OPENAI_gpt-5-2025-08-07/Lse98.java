import java.security.SecureRandom;
import java.security.MessageDigest;
import java.util.Base64;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.util.concurrent.ConcurrentHashMap;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

/*
Chain-of-Through process in code generation (documented as comments inside code):
1. Problem understanding: Implement a simple registration page logic that accepts a username and password, hashes the password, and stores it in a database (simulated in-memory).
2. Security requirements: Use strong password hashing (PBKDF2-HMAC-SHA256 with salt and iterations), input validation, constant-time comparisons, avoid storing plaintext passwords, and clear sensitive memory.
3. Secure coding generation: Implement safe validators, secure random salt, PBKDF2 hashing, structured storage format, uniqueness checks, and no sensitive logging.
4. Code review: Ensure no plaintext password exposure, use constant-time comparison, unique username enforcement, secure defaults, and parameterized interfaces.
5. Secure code output: Final version below mitigates identified risks.
*/

public class Lse98 {

    // Simple in-memory "database" abstraction
    public static final class SecureDatabase {
        private final ConcurrentHashMap<String, String> users = new ConcurrentHashMap<>();

        public boolean insertUser(String username, String passwordRecord) {
            // Prevent overwrite: putIfAbsent returns null if absent
            return users.putIfAbsent(username, passwordRecord) == null;
        }

        public String getPasswordRecord(String username) {
            return users.get(username);
        }
    }

    // Configuration
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32; // 256-bit
    private static final int ITERATIONS = 120_000; // reasonable default
    private static final SecureRandom RNG = new SecureRandom();

    // Registration API: accept parameters and return output
    public static boolean register(SecureDatabase db, String username, char[] password) {
        if (db == null || username == null || password == null) {
            safeZero(password);
            return false;
        }
        try {
            if (!isValidUsername(username) || !isValidPassword(password)) {
                safeZero(password);
                return false;
            }

            String record = hashPassword(password, ITERATIONS);
            // password cleared by hashPassword
            return db.insertUser(username, record);
        } finally {
            safeZero(password);
        }
    }

    public static boolean verify(SecureDatabase db, String username, char[] password) {
        if (db == null || username == null || password == null) {
            safeZero(password);
            return false;
        }
        try {
            String record = db.getPasswordRecord(username);
            if (record == null) return false;
            return verifyPassword(password, record);
        } finally {
            safeZero(password);
        }
    }

    private static boolean isValidUsername(String username) {
        if (username.length() < 3 || username.length() > 32) return false;
        // Allowed: letters, digits, underscore, hyphen, dot
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         c == '_' || c == '-' || c == '.';
            if (!ok) return false;
        }
        return true;
    }

    private static boolean isValidPassword(char[] password) {
        if (password.length < 8 || password.length > 128) return false;
        // Simple checks: at least one letter and one digit
        boolean hasLetter = false, hasDigit = false;
        for (char c : password) {
            if (Character.isLetter(c)) hasLetter = true;
            if (Character.isDigit(c)) hasDigit = true;
        }
        return hasLetter && hasDigit;
    }

    private static String hashPassword(char[] password, int iterations) {
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);

        byte[] hash = null;
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, HASH_LEN * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            hash = skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            // On error, ensure not to leak info
            throw new RuntimeException("Hashing failed");
        } finally {
            safeZero(password);
        }

        String saltB64 = Base64.getEncoder().encodeToString(salt);
        String hashB64 = Base64.getEncoder().encodeToString(hash);
        Arrays.fill(hash, (byte)0);

        // Record format: v=1$iterations$salt$hash
        return "v=1$" + iterations + "$" + saltB64 + "$" + hashB64;
    }

    private static boolean verifyPassword(char[] password, String record) {
        try {
            String[] parts = record.split("\\$");
            if (parts.length != 4) return false;
            // parts[0] = "v=1"
            if (!"v=1".equals(parts[0])) return false;
            int iterations = Integer.parseInt(parts[1]);
            byte[] salt = Base64.getDecoder().decode(parts[2]);
            byte[] expected = Base64.getDecoder().decode(parts[3]);

            byte[] actual;
            try {
                PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, expected.length * 8);
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                actual = skf.generateSecret(spec).getEncoded();
            } catch (Exception e) {
                return false;
            }

            boolean eq = constantTimeEquals(expected, actual);
            Arrays.fill(actual, (byte)0);
            Arrays.fill(expected, (byte)0);
            Arrays.fill(salt, (byte)0);
            return eq;
        } catch (Exception e) {
            return false;
        } finally {
            safeZero(password);
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) {
            r |= (a[i] ^ b[i]);
        }
        return r == 0;
    }

    private static void safeZero(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        SecureDatabase db = new SecureDatabase();

        String[] usernames = {
            "alice",
            "bob",
            "alice",           // duplicate
            "invalid user",    // invalid (space)
            "charlie"
        };
        String[] passwords = {
            "Password123!",
            "S3curePa$$word",
            "AnotherPass1!",
            "Password123!",
            "short"            // invalid (too short)
        };

        for (int i = 0; i < 5; i++) {
            char[] pw = passwords[i].toCharArray();
            boolean ok = register(db, usernames[i], pw);
            System.out.println("Register(" + usernames[i] + "): " + ok);
        }

        // Additional verification checks
        System.out.println("Verify bob correct: " + verify(db, "bob", "S3curePa$$word".toCharArray()));
        System.out.println("Verify bob wrong: " + verify(db, "bob", "WrongPass123".toCharArray()));
        System.out.println("Verify alice correct: " + verify(db, "alice", "Password123!".toCharArray()));
        System.out.println("Verify alice wrong: " + verify(db, "alice", "AnotherPass1!".toCharArray()));
        System.out.println("Verify unknown: " + verify(db, "unknown", "Whatever123".toCharArray()));
    }
}