import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse38 {
    // Step 1: Problem understanding
    // - Input: username, password
    // - Output: redirect path "/dashboard" if correct, "/login" if incorrect
    // Step 2: Security requirements
    // - Store hashed passwords with PBKDF2 (HMAC-SHA256), per-user salt, adequate iterations
    // - Constant-time comparison to mitigate timing attacks
    // - No SQL; in-memory map to simulate database (avoids injection)
    // Step 3: Secure coding generation (implementation below)
    // Step 4: Code review (performed inline with careful API use)
    // Step 5: Secure code output (finalized implementation)

    private static final int ITERATIONS = 100_000;
    private static final int KEY_LENGTH = 256; // bits
    private static final Map<String, UserRecord> USER_DB = new HashMap<>();

    private static final class UserRecord {
        final byte[] salt;
        final int iterations;
        final byte[] hash;

        UserRecord(byte[] salt, int iterations, byte[] hash) {
            this.salt = salt.clone();
            this.iterations = iterations;
            this.hash = hash.clone();
        }
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLengthBits) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLengthBits);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
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

    // Initialize the "database" with two users and hashed passwords.
    private static void initUserDb() {
        try {
            // For demonstration, salts are fixed constants; in real systems, generate per user with SecureRandom.
            byte[] aliceSalt = "ALICE_SALT".getBytes(java.nio.charset.StandardCharsets.UTF_8);
            byte[] bobSalt = "BOB_SALT".getBytes(java.nio.charset.StandardCharsets.UTF_8);

            byte[] aliceHash = pbkdf2("alicePass123!".toCharArray(), aliceSalt, ITERATIONS, KEY_LENGTH);
            byte[] bobHash = pbkdf2("bobSecure#456".toCharArray(), bobSalt, ITERATIONS, KEY_LENGTH);

            USER_DB.put("alice", new UserRecord(aliceSalt, ITERATIONS, aliceHash));
            USER_DB.put("bob", new UserRecord(bobSalt, ITERATIONS, bobHash));
        } catch (Exception e) {
            throw new RuntimeException("Failed to initialize user database securely", e);
        }
    }

    // Main login processing function
    public static String processLogin(String username, String password) {
        if (username == null || password == null) return "/login";
        String uname = username.trim();
        if (uname.isEmpty()) return "/login";

        UserRecord rec = USER_DB.get(uname);
        if (rec == null) return "/login";
        try {
            byte[] candidate = pbkdf2(password.toCharArray(), rec.salt, rec.iterations, KEY_LENGTH);
            if (constantTimeEquals(candidate, rec.hash)) {
                // Successful authentication
                return "/dashboard";
            }
        } catch (Exception e) {
            // On error, fail securely
        }
        return "/login";
    }

    public static void main(String[] args) {
        initUserDb();

        // 5 test cases
        String[][] tests = new String[][]{
            {"alice", "alicePass123!"},
            {"alice", "wrongPassword"},
            {"unknown", "somepass"},
            {"bob", "bobSecure#456"},
            {"", "any"}
        };

        for (String[] t : tests) {
            String result = processLogin(t[0], t[1]);
            System.out.println("Login(" + t[0] + ", ****) -> " + result);
        }
    }
}