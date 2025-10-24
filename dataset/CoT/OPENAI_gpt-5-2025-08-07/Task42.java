import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/*
Chain-of-Through process:
1) Problem understanding:
   - Implement a simple user authentication system that securely handles user credentials.
   - Support registering users with a password and authenticating given username and password.
   - In-memory store for simplicity.

2) Security requirements:
   - Use strong password hashing (PBKDF2-HMAC-SHA256) with a per-user random salt and high iteration count.
   - Constant-time comparison for hashes.
   - Avoid leaking secrets; minimize lifetime of sensitive data.

3) Secure coding generation:
   - Generate salt with SecureRandom.
   - Store salt, iteration count, and derived key (hash).
   - Validate inputs (non-empty username, basic password policy).
   - No plaintext password storage.
   - No global mutable state exposed.

4) Code review:
   - Use MessageDigest.isEqual for constant-time compare.
   - Use PBEKeySpec with char[] to avoid String secrets.
   - Clear PBEKeySpec and temporary arrays where possible.

5) Secure code output:
   - Final code below addresses above requirements and includes 5 test cases.
*/
public class Task42 {
    static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    static final class UserAuth {
        private final Map<String, UserRecord> db = new ConcurrentHashMap<>();
        private static final int SALT_BYTES = 16;
        private static final int HASH_BYTES = 32;
        private static final int ITERATIONS = 200_000;

        public boolean registerUser(String username, char[] password) {
            if (!isValidUsername(username) || !isValidPassword(password)) {
                return false;
            }
            if (db.containsKey(username)) {
                return false;
            }
            byte[] salt = new byte[SALT_BYTES];
            byte[] hash = null;
            try {
                SecureRandom sr;
                try {
                    sr = SecureRandom.getInstanceStrong();
                } catch (Exception e) {
                    sr = new SecureRandom();
                }
                sr.nextBytes(salt);
                hash = pbkdf2(password, salt, ITERATIONS, HASH_BYTES);
                if (hash == null) return false;
                db.put(username, new UserRecord(Arrays.copyOf(salt, salt.length), Arrays.copyOf(hash, hash.length), ITERATIONS));
                return true;
            } finally {
                if (hash != null) Arrays.fill(hash, (byte)0);
                Arrays.fill(salt, (byte)0);
            }
        }

        public boolean authenticate(String username, char[] password) {
            UserRecord rec = db.get(username);
            if (rec == null) return false;
            byte[] computed = null;
            try {
                computed = pbkdf2(password, rec.salt, rec.iterations, rec.hash.length);
                if (computed == null) return false;
                return MessageDigest.isEqual(computed, rec.hash);
            } finally {
                if (computed != null) Arrays.fill(computed, (byte)0);
            }
        }

        private static boolean isValidUsername(String u) {
            if (u == null) return false;
            if (u.length() < 1 || u.length() > 64) return false;
            // Simple policy: letters, digits, underscore, dash, dot
            for (int i = 0; i < u.length(); i++) {
                char c = u.charAt(i);
                if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.')) return false;
            }
            return true;
        }

        private static boolean isValidPassword(char[] p) {
            if (p == null || p.length < 8) return false;
            return true;
        }

        private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLengthBytes) {
            PBEKeySpec spec = null;
            try {
                spec = new PBEKeySpec(password, salt, iterations, keyLengthBytes * 8);
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                return skf.generateSecret(spec).getEncoded();
            } catch (Exception e) {
                return null;
            } finally {
                if (spec != null) spec.clearPassword();
            }
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        UserAuth auth = new UserAuth();

        char[] pwAlice = "CorrectHorse1!".toCharArray();
        char[] pwAliceWrong = "WrongHorse1!".toCharArray();
        char[] pwBob = "S3cureBob#2025".toCharArray();
        char[] pwCarol = "Another$Pass9".toCharArray();

        boolean t1 = auth.registerUser("alice", pwAlice) && auth.authenticate("alice", pwAlice);
        boolean t2 = !auth.authenticate("alice", pwAliceWrong);
        boolean t3 = !auth.registerUser("alice", pwAlice); // duplicate
        boolean t4 = auth.registerUser("bob-01", pwBob) && auth.authenticate("bob-01", pwBob);
        boolean t5 = !auth.authenticate("eve", pwCarol);

        System.out.println("Test1 register+auth alice (expect true): " + t1);
        System.out.println("Test2 wrong password (expect true): " + t2);
        System.out.println("Test3 duplicate register (expect true): " + t3);
        System.out.println("Test4 register+auth bob (expect true): " + t4);
        System.out.println("Test5 non-existent user (expect true): " + t5);

        // Clean sensitive arrays used in tests
        Arrays.fill(pwAlice, '\0');
        Arrays.fill(pwAliceWrong, '\0');
        Arrays.fill(pwBob, '\0');
        Arrays.fill(pwCarol, '\0');
    }
}