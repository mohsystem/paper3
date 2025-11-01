import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class Lse35 {

    private static final int ITERATIONS = 150_000;
    private static final int KEY_LENGTH_BITS = 256; // 32 bytes
    private static final int SALT_LENGTH_BYTES = 16;
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    private static final Map<String, UserRecord> DATABASE =
            Collections.synchronizedMap(new HashMap<>());

    private static class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        final int keyLengthBits;

        UserRecord(byte[] salt, byte[] hash, int iterations, int keyLengthBits) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
            this.keyLengthBits = keyLengthBits;
        }
    }

    public static void addUser(String username, String password) {
        if (!isValidUsername(username) || !isValidPassword(password)) {
            return;
        }
        byte[] salt = new byte[SALT_LENGTH_BYTES];
        SECURE_RANDOM.nextBytes(salt);
        byte[] hash = deriveKey(password, salt, ITERATIONS, KEY_LENGTH_BITS);
        DATABASE.put(username, new UserRecord(salt, hash, ITERATIONS, KEY_LENGTH_BITS));
    }

    public static String login(String username, String password) {
        if (!isValidUsername(username) || !isValidPassword(password)) {
            return "login.html";
        }
        UserRecord rec = DATABASE.get(username);
        if (rec == null) {
            // Avoid leaking existence by doing a dummy derive and compare with a dummy hash
            byte[] dummySalt = new byte[SALT_LENGTH_BYTES];
            byte[] dummyHash = new byte[KEY_LENGTH_BITS / 8];
            SECURE_RANDOM.nextBytes(dummySalt);
            byte[] candidate = deriveKey(password, dummySalt, ITERATIONS, KEY_LENGTH_BITS);
            constantTimeEquals(candidate, dummyHash);
            return "login.html";
        }
        byte[] candidate = deriveKey(password, rec.salt, rec.iterations, rec.keyLengthBits);
        if (constantTimeEquals(candidate, rec.hash)) {
            return "home.html";
        }
        return "login.html";
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        int len = username.length();
        if (len < 1 || len > 128) return false;
        // Basic sanity: allow alnum and a few safe symbols
        for (int i = 0; i < len; i++) {
            char c = username.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.' || c == '@')) {
                return false;
            }
        }
        return true;
    }

    private static boolean isValidPassword(String password) {
        if (password == null) return false;
        int len = password.length();
        return len >= 1 && len <= 256;
    }

    private static byte[] deriveKey(String password, byte[] salt, int iterations, int keyLengthBits) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, iterations, keyLengthBits);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (GeneralSecurityException e) {
            // Fallback: never expose details; in failure, return random bytes to avoid oracle
            byte[] fallback = new byte[keyLengthBits / 8];
            SECURE_RANDOM.nextBytes(fallback);
            return fallback;
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        int len = Math.max(a.length, b.length);
        int result = 0;
        for (int i = 0; i < len; i++) {
            byte x = i < a.length ? a[i] : 0;
            byte y = i < b.length ? b[i] : 0;
            result |= (x ^ y);
        }
        return result == 0 && a.length == b.length;
    }

    // Helper to show stored data without leaking secrets (demonstration only)
    private static String b64(byte[] data) {
        return Base64.getEncoder().encodeToString(data);
    }

    public static void main(String[] args) {
        // Initialize "database" with some users
        addUser("alice", "Correct#Horse1");
        addUser("bob", "S3cure-Pass!");
        addUser("charlie", "p@$$w0rd!");

        // 5 Test cases
        String[][] tests = new String[][]{
                {"alice", "Correct#Horse1"},     // valid
                {"alice", "wrong"},              // invalid password
                {"bob", "S3cure-Pass!"},         // valid
                {"unknown", "anything"},         // non-existent user
                {"", ""}                         // invalid inputs
        };

        for (int i = 0; i < tests.length; i++) {
            String user = tests[i][0];
            String pass = tests[i][1];
            String result = login(user, pass);
            System.out.println("Test " + (i + 1) + ": user=" + user + " => " + result);
        }
    }
}