import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse34 {
    private static final int ITERATIONS = 100_000;
    private static final int KEY_LEN = 32; // 256-bit
    private static final int SALT_LEN = 16;
    private static final SecureRandom RNG = new SecureRandom();

    private static class UserRecord {
        final String username;
        final byte[] salt;
        final byte[] hash;
        UserRecord(String username, byte[] salt, byte[] hash) {
            this.username = username;
            this.salt = salt;
            this.hash = hash;
        }
    }

    private static final Map<String, UserRecord> DB = new HashMap<>();

    public static void addUser(String username, String password) {
        if (username == null || password == null) return;
        String u = safeTrim(username);
        if (u.isEmpty()) return;
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] hash = pbkdf2(password.toCharArray(), salt);
        DB.put(u, new UserRecord(u, salt, hash));
    }

    private static byte[] pbkdf2(char[] password, byte[] salt) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] key = skf.generateSecret(spec).getEncoded();
            Arrays.fill(password, '\0');
            return key;
        } catch (Exception e) {
            Arrays.fill(password, '\0');
            throw new RuntimeException("KDF error", e);
        }
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

    private static String safeTrim(String s) {
        return s == null ? "" : s.trim();
    }

    public static String login(String username, String password) {
        String u = safeTrim(username);
        if (u.isEmpty() || password == null) {
            return "redirect:/login";
        }
        UserRecord rec = DB.get(u);
        if (rec == null) {
            return "redirect:/login";
        }
        byte[] cand = pbkdf2(password.toCharArray(), rec.salt);
        boolean ok = constantTimeEquals(cand, rec.hash);
        Arrays.fill(cand, (byte) 0);
        return ok ? "redirect:/dashboard" : "redirect:/login";
    }

    public static void main(String[] args) {
        addUser("alice", "CorrectHorseBatteryStaple!");
        addUser("bob", "b0b$ecret");
        addUser("eve", "n0t_used");

        // 5 test cases
        System.out.println("Test1 (alice correct): " + login("alice", "CorrectHorseBatteryStaple!")); // dashboard
        System.out.println("Test2 (alice wrong pw): " + login("alice", "wrong")); // login
        System.out.println("Test3 (unknown user): " + login("charlie", "anything")); // login
        System.out.println("Test4 (empty username): " + login("   ", "whatever")); // login
        System.out.println("Test5 (bob correct with trim): " + login("  bob ", "b0b$ecret")); // dashboard
    }
}