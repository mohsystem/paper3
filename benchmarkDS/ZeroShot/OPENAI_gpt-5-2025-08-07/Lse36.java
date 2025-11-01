import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse36 {
    private static final int ITERATIONS = 120000;
    private static final int KEY_LEN = 32; // 256-bit
    private static final int MAX_USERNAME_LEN = 64;
    private static final int MAX_PASSWORD_LEN = 128;

    private static final Map<String, UserRecord> db = new HashMap<>();

    private static final byte[] DUMMY_SALT = new byte[] {
        (byte)0xA3, (byte)0x51, (byte)0x5C, (byte)0x09, (byte)0x7E, (byte)0x22, (byte)0xA9, (byte)0x41,
        (byte)0x13, (byte)0xF1, (byte)0xB2, (byte)0x08, (byte)0xCD, (byte)0x44, (byte)0x9A, (byte)0x7F
    };

    private static class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    public static String login(String username, String password) {
        if (username == null || password == null) return null;
        if (username.isBlank() || username.length() > MAX_USERNAME_LEN) return null;
        if (password.length() > MAX_PASSWORD_LEN) return null;

        UserRecord rec = db.get(username);
        if (rec == null) {
            // Perform dummy work to mitigate user enumeration timing
            try {
                byte[] dummy = pbkdf2(password.toCharArray(), DUMMY_SALT, ITERATIONS, KEY_LEN);
                // constant-time compare with itself to consume same time
                constantTimeEquals(dummy, dummy);
                Arrays.fill(dummy, (byte)0);
            } catch (Exception ignored) {}
            return null;
        }

        try {
            byte[] computed = pbkdf2(password.toCharArray(), rec.salt, rec.iterations, KEY_LEN);
            boolean ok = constantTimeEquals(computed, rec.hash);
            Arrays.fill(computed, (byte)0);
            return ok ? "/home" : null;
        } catch (Exception e) {
            return null;
        }
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        KeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] out = skf.generateSecret(spec).getEncoded();
        Arrays.fill(password, '\0'); // clear
        return out;
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        int len = Math.max(a.length, b.length);
        int result = 0;
        for (int i = 0; i < len; i++) {
            byte x = (i < a.length) ? a[i] : 0;
            byte y = (i < b.length) ? b[i] : 0;
            result |= (x ^ y);
        }
        return result == 0 && a.length == b.length;
    }

    private static void addUser(String username, String password, byte[] salt, int iterations) {
        try {
            byte[] hash = pbkdf2(password.toCharArray(), salt, iterations, KEY_LEN);
            db.put(username, new UserRecord(Arrays.copyOf(salt, salt.length), hash, iterations));
        } catch (Exception e) {
            throw new RuntimeException("Failed to add user");
        }
    }

    private static void initializeDb() {
        // Fixed salt for deterministic demo; in real systems use SecureRandom per user.
        byte[] saltAlice = new byte[] {
            (byte)0x10, (byte)0x2F, (byte)0x3A, (byte)0x4B, (byte)0x5C, (byte)0x6D, (byte)0x7E, (byte)0x8F,
            (byte)0x90, (byte)0xAB, (byte)0xBC, (byte)0xCD, (byte)0xDE, (byte)0xEF, (byte)0x01, (byte)0x23
        };
        addUser("alice", "correct horse battery staple", saltAlice, ITERATIONS);

        byte[] saltBob = new byte[] {
            (byte)0x33, (byte)0x55, (byte)0x77, (byte)0x99, (byte)0xAA, (byte)0xBB, (byte)0xCC, (byte)0xDD,
            (byte)0xEE, (byte)0xFF, (byte)0x12, (byte)0x34, (byte)0x56, (byte)0x78, (byte)0x9A, (byte)0xBC
        };
        addUser("bob", "Tr0ub4dor&3", saltBob, ITERATIONS);
    }

    private static void runTest(String label, String username, String password) {
        String res = login(username, password);
        System.out.println(label + ": " + ("/home".equals(res) ? "redirect:/home" : "no-redirect"));
    }

    public static void main(String[] args) {
        initializeDb();
        runTest("Test1 - correct credentials", "alice", "correct horse battery staple");
        runTest("Test2 - wrong password", "alice", "wrong password");
        runTest("Test3 - unknown user", "charlie", "anything");
        runTest("Test4 - empty username", "", "any");
        runTest("Test5 - injection-like username", "alice' OR '1'='1", "foo");
    }
}