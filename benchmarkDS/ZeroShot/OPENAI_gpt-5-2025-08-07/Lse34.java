import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse34 {

    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 100_000;
    private static final int KEY_LEN = 32;

    static class UserRecord {
        final String username;
        final byte[] salt;
        final byte[] hash;
        UserRecord(String username, byte[] salt, byte[] hash) {
            this.username = username;
            this.salt = salt;
            this.hash = hash;
        }
    }

    public static UserRecord createUserRecord(String username, char[] password) {
        if (username == null || password == null) throw new IllegalArgumentException("Invalid input");
        byte[] salt = new byte[SALT_LEN];
        new SecureRandom().nextBytes(salt);
        byte[] hash = deriveKey(password, salt, ITERATIONS, KEY_LEN);
        Arrays.fill(password, '\0');
        return new UserRecord(username, salt, hash);
    }

    private static byte[] deriveKey(char[] password, byte[] salt, int iterations, int keyLen) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] out = skf.generateSecret(spec).getEncoded();
            spec.clearPassword();
            return out;
        } catch (GeneralSecurityException e) {
            throw new RuntimeException("KDF failed");
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    public static String authenticateAndRedirect(Map<String, UserRecord> db, String username, char[] password) {
        try {
            if (db == null || username == null || password == null) return "redirect:/login";
            UserRecord rec = db.get(username);
            if (rec == null) return "redirect:/login";
            byte[] computed = deriveKey(password, rec.salt, ITERATIONS, KEY_LEN);
            boolean ok = constantTimeEquals(rec.hash, computed);
            Arrays.fill(computed, (byte) 0);
            return ok ? "redirect:/dashboard" : "redirect:/login";
        } finally {
            if (password != null) Arrays.fill(password, '\0');
        }
    }

    public static void main(String[] args) {
        Map<String, UserRecord> db = new HashMap<>();
        db.put("alice", createUserRecord("alice", "S3curePass!".toCharArray()));
        db.put("bob", createUserRecord("bob", "hunter2?".toCharArray()));

        System.out.println(authenticateAndRedirect(db, "alice", "S3curePass!".toCharArray())); // dashboard
        System.out.println(authenticateAndRedirect(db, "alice", "wrongpass".toCharArray()));    // login
        System.out.println(authenticateAndRedirect(db, "unknown", "anything".toCharArray()));   // login
        System.out.println(authenticateAndRedirect(db, "bob", "hunter2?".toCharArray()));       // dashboard
        System.out.println(authenticateAndRedirect(db, "", "".toCharArray()));                  // login
    }
}