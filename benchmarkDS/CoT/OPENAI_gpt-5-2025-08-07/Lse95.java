import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/*
Chain-of-Through process:
1) Problem understanding:
   - Build a simple in-memory "database" for registration/login.
   - On registration: validate inputs, generate random salt, derive password hash (PBKDF2-HMAC-SHA256), store "hex(hash):hex(salt)" per user.
   - On login: fetch user's record, recompute hash with stored salt, constant-time compare.
2) Security requirements:
   - Use SecureRandom for salts.
   - Use PBKDF2WithHmacSHA256 with sufficient iterations and key length.
   - Constant-time comparison for verifying hashes.
   - Validate inputs and prevent duplicate usernames.
3) Secure coding generation: implemented below.
4) Code review: ensure no sensitive info is logged, parameters validated, safe conversions.
5) Secure code output: final, fixed and safe.
*/

public class Lse95 {
    public static class SecureUserStore {
        private final Map<String, String> db = new ConcurrentHashMap<>();
        private static final int SALT_LEN = 16;
        private static final int ITERATIONS = 120_000;
        private static final int KEY_LEN_BITS = 256;

        public boolean registerUser(String username, char[] password) throws Exception {
            if (!isValidUsername(username) || !isValidPassword(password)) {
                return false;
            }
            if (db.containsKey(username)) {
                return false;
            }
            byte[] salt = generateSalt(SALT_LEN);
            byte[] hash = pbkdf2(password, salt, ITERATIONS, KEY_LEN_BITS);
            Arrays.fill(password, '\0');
            String record = toHex(hash) + ":" + toHex(salt);
            db.put(username, record);
            return true;
        }

        public boolean verifyLogin(String username, char[] password) throws Exception {
            String record = db.get(username);
            if (record == null) {
                Arrays.fill(password, '\0');
                return false;
            }
            String[] parts = record.split(":");
            if (parts.length != 2) {
                Arrays.fill(password, '\0');
                return false;
            }
            byte[] storedHash = fromHex(parts[0]);
            byte[] salt = fromHex(parts[1]);
            byte[] computed = pbkdf2(password, salt, ITERATIONS, KEY_LEN_BITS);
            Arrays.fill(password, '\0');
            boolean ok = MessageDigest.isEqual(storedHash, computed);
            Arrays.fill(computed, (byte) 0);
            return ok;
        }

        private static boolean isValidUsername(String u) {
            if (u == null) return false;
            if (u.length() < 3 || u.length() > 64) return false;
            return u.matches("^[A-Za-z0-9_.-]+$");
        }

        private static boolean isValidPassword(char[] p) {
            if (p == null || p.length < 8 || p.length > 1024) return false;
            for (char c : p) {
                if (Character.isISOControl(c)) return false;
            }
            return true;
        }

        private static byte[] generateSalt(int len) {
            byte[] salt = new byte[len];
            new SecureRandom().nextBytes(salt);
            return salt;
        }

        private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLenBits) throws Exception {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLenBits);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        }

        private static String toHex(byte[] data) {
            StringBuilder sb = new StringBuilder(data.length * 2);
            for (byte b : data) {
                sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
                sb.append(Character.forDigit((b) & 0xF, 16));
            }
            return sb.toString();
        }

        private static byte[] fromHex(String s) {
            int len = s.length();
            byte[] out = new byte[len / 2];
            for (int i = 0; i < len; i += 2) {
                int hi = Character.digit(s.charAt(i), 16);
                int lo = Character.digit(s.charAt(i + 1), 16);
                out[i / 2] = (byte) ((hi << 4) | lo);
            }
            return out;
        }
    }

    public static void main(String[] args) throws Exception {
        SecureUserStore store = new SecureUserStore();

        // Test case 1: Register new user
        System.out.println("TC1 register alice: " + store.registerUser("alice", "S3curePwd!".toCharArray()));

        // Test case 2: Duplicate registration should fail
        System.out.println("TC2 register alice again: " + store.registerUser("alice", "AnotherPwd!".toCharArray()));

        // Test case 3: Correct login
        System.out.println("TC3 login alice correct: " + store.verifyLogin("alice", "S3curePwd!".toCharArray()));

        // Test case 4: Wrong password
        System.out.println("TC4 login alice wrong: " + store.verifyLogin("alice", "WrongPass!".toCharArray()));

        // Test case 5: Another user register and login
        System.out.println("TC5 register bob: " + store.registerUser("bob-01", "P@ssw0rd123".toCharArray()));
        System.out.println("TC5 login bob correct: " + store.verifyLogin("bob-01", "P@ssw0rd123".toCharArray()));
    }
}