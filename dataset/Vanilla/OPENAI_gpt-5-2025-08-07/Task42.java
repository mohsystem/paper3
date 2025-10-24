import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task42 {

    static class User {
        final String username;
        final byte[] salt;
        final byte[] hash;
        User(String username, byte[] salt, byte[] hash) {
            this.username = username;
            this.salt = salt;
            this.hash = hash;
        }
    }

    private final Map<String, User> users = new HashMap<>();

    public boolean registerUser(String username, String password) {
        if (users.containsKey(username)) {
            return false;
        }
        try {
            byte[] salt = new byte[16];
            SecureRandom sr = SecureRandom.getInstanceStrong();
            sr.nextBytes(salt);
            byte[] hash = pbkdf2(password.toCharArray(), salt, 100_000, 32);
            users.put(username, new User(username, salt, hash));
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public boolean authenticate(String username, String password) {
        try {
            User u = users.get(username);
            if (u == null) return false;
            byte[] test = pbkdf2(password.toCharArray(), u.salt, 100_000, 32);
            return MessageDigest.isEqual(u.hash, test);
        } catch (Exception e) {
            return false;
        }
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    public static void main(String[] args) {
        Task42 auth = new Task42();
        System.out.println(auth.registerUser("alice", "password123")); // Test 1
        System.out.println(auth.registerUser("bob", "S3cur3!"));       // Test 2
        System.out.println(auth.registerUser("alice", "another"));     // Test 3 (duplicate)
        System.out.println(auth.authenticate("alice", "password123")); // Test 4
        System.out.println(auth.authenticate("alice", "wrong"));       // Test 5
    }
}