import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse38 {

    static final int SALT_LEN = 16;
    static final int HASH_LEN = 32; // 256-bit
    static final int ITERATIONS = 210_000;

    static class User {
        final String username;
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        User(String username, byte[] salt, byte[] hash, int iterations) {
            this.username = username;
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
            this.iterations = iterations;
        }
    }

    static class InMemoryDB {
        private final Map<String, User> users = new HashMap<>();

        void addUser(String username, String password) throws Exception {
            if (!validateUsername(username)) {
                throw new IllegalArgumentException("Invalid username format");
            }
            if (!validatePassword(password)) {
                throw new IllegalArgumentException("Invalid password format");
            }
            if (users.containsKey(username)) {
                throw new IllegalArgumentException("User exists");
            }
            byte[] salt = new byte[SALT_LEN];
            SecureRandom sr = SecureRandom.getInstanceStrong();
            sr.nextBytes(salt);
            byte[] hash = pbkdf2(password.toCharArray(), salt, ITERATIONS, HASH_LEN);
            users.put(username, new User(username, salt, hash, ITERATIONS));
            // Best-effort clear of password char[] via PBEKeySpec later; here string immutable
        }

        User findByUsername(String username) {
            return users.get(username);
        }
    }

    static boolean validateUsername(String username) {
        if (username == null) return false;
        if (username.length() < 3 || username.length() > 32) return false;
        return username.matches("^[A-Za-z0-9._-]+$");
    }

    static boolean validatePassword(String password) {
        if (password == null) return false;
        int len = password.length();
        return len >= 8 && len <= 128;
    }

    static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] out = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return out;
    }

    static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    // Core login function
    public static String handleLogin(InMemoryDB db, String username, String password) {
        try {
            if (!validateUsername(username) || !validatePassword(password)) {
                return "REDIRECT:/login";
            }
            User u = db.findByUsername(username);
            if (u == null) {
                return "REDIRECT:/login";
            }
            byte[] computed = pbkdf2(password.toCharArray(), u.salt, u.iterations, u.hash.length);
            boolean ok = constantTimeEquals(computed, u.hash);
            Arrays.fill(computed, (byte) 0);
            return ok ? "REDIRECT:/dashboard" : "REDIRECT:/login";
        } catch (Exception e) {
            return "REDIRECT:/login";
        }
    }

    public static void main(String[] args) throws Exception {
        InMemoryDB db = new InMemoryDB();
        // Create test users
        db.addUser("alice", "CorrectHorse1!");
        db.addUser("bob", "S3cureP@ssw0rd");

        // 5 test cases
        String[] results = new String[5];
        results[0] = handleLogin(db, "alice", "CorrectHorse1!"); // correct
        results[1] = handleLogin(db, "alice", "wrongpass");      // wrong password
        results[2] = handleLogin(db, "charlie", "NoSuchUser1!"); // unknown user
        results[3] = handleLogin(db, "x", "SomeValid1!");        // invalid username (too short)
        results[4] = handleLogin(db, "bob", "short");            // invalid password (too short)

        for (int i = 0; i < results.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + results[i]);
        }
    }
}