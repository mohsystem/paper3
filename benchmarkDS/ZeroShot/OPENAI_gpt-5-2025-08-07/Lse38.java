import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse38 {
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 150000;
    private static final int KEY_LEN = 256; // bits
    private static final Map<String, User> DB = new HashMap<>();

    private static class User {
        final byte[] salt;
        final byte[] hash;
        User(byte[] s, byte[] h) { this.salt = s; this.hash = h; }
    }

    public static boolean addUser(String username, char[] password) {
        if (username == null || username.isEmpty() || password == null) return false;
        if (DB.containsKey(username)) { zeroCharArray(password); return false; }
        try {
            byte[] salt = new byte[SALT_LEN];
            SecureRandom sr = SecureRandom.getInstanceStrong();
            sr.nextBytes(salt);
            byte[] hash = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
            DB.put(username, new User(salt, hash));
            zeroCharArray(password);
            return true;
        } catch (Exception e) {
            zeroCharArray(password);
            return false;
        }
    }

    public static String login(String username, char[] password) {
        try {
            if (username == null || password == null) return "/login";
            User u = DB.get(username);
            if (u == null) {
                zeroCharArray(password);
                return "/login";
            }
            byte[] computed = pbkdf2(password, u.salt, ITERATIONS, KEY_LEN);
            zeroCharArray(password);
            return MessageDigest.isEqual(computed, u.hash) ? "/dashboard" : "/login";
        } catch (Exception e) {
            zeroCharArray(password);
            return "/login";
        }
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLenBits) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLenBits);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] out = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return out;
    }

    private static void zeroCharArray(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    public static void main(String[] args) {
        addUser("alice", "Correct$Horse1".toCharArray());
        addUser("bob", "S3curePass!".toCharArray());

        System.out.println(login("alice", "Correct$Horse1".toCharArray())); // expected /dashboard
        System.out.println(login("alice", "wrongpass".toCharArray()));       // expected /login
        System.out.println(login("unknown", "whatever".toCharArray()));      // expected /login
        System.out.println(login("", "".toCharArray()));                      // expected /login
        System.out.println(login("bob", "S3curePass!".toCharArray()));       // expected /dashboard
    }
}