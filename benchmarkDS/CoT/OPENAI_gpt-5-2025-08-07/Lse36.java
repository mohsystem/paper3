import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse36 {
    private static final String HOME_URL = "/home";
    private static final int ITERATIONS = 100_000;
    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 256; // bits

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

    private static final Map<String, UserRecord> DB = new HashMap<>();
    private static final SecureRandom RNG = new SecureRandom();

    // Securely adds a user to the in-memory "database"
    public static void addUser(String username, char[] password) {
        if (username == null || username.isBlank() || password == null) return;
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] hash = deriveKey(password, salt, ITERATIONS);
        zeroCharArray(password);
        DB.put(username, new UserRecord(salt, hash, ITERATIONS));
    }

    // Derives a key using PBKDF2-HMAC-SHA256
    private static byte[] deriveKey(char[] password, byte[] salt, int iterations) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, iterations, KEY_LEN);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            throw new RuntimeException("Key derivation failed", e);
        }
    }

    private static void zeroCharArray(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    // Constant-time comparison
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    // The login function: returns redirect URL if success, otherwise null
    public static String login(String username, char[] password) {
        if (username == null || password == null) {
            if (password != null) zeroCharArray(password);
            return null;
        }
        UserRecord rec = DB.get(username);
        if (rec == null) {
            zeroCharArray(password);
            return null;
        }
        byte[] computed = deriveKey(password, rec.salt, rec.iterations);
        zeroCharArray(password);
        if (constantTimeEquals(computed, rec.hash)) {
            return HOME_URL;
        }
        return null;
    }

    public static void main(String[] args) {
        // Setup users
        addUser("alice", "Password123!".toCharArray());
        addUser("bob", "s3cur3".toCharArray());

        // 5 test cases
        String[] usernames = {
            "alice",    // correct
            "alice",    // wrong password
            "bob",      // correct
            "charlie",  // non-existent
            ""          // empty username
        };
        String[] passwords = {
            "Password123!",
            "wrong",
            "s3cur3",
            "anything",
            ""
        };

        for (int i = 0; i < 5; i++) {
            String redirect = login(usernames[i], passwords[i].toCharArray());
            System.out.println("Test " + (i + 1) + ": " + (redirect != null ? redirect : "login failed"));
        }
    }
}