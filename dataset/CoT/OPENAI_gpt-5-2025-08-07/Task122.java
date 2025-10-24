import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task122 {

    static final class SecureUserStore {
        private final Map<String, String> users = new ConcurrentHashMap<>();
        private static final String KDF_ALGO = "PBKDF2WithHmacSHA256";
        private static final int ITERATIONS = 200_000;
        private static final int SALT_LEN = 16;
        private static final int KEY_LEN_BYTES = 32;
        private static final SecureRandom RNG = new SecureRandom();

        public boolean signup(String username, char[] password) {
            if (!validateUsername(username) || !validatePassword(password)) {
                zeroize(password);
                return false;
            }
            if (users.containsKey(username)) {
                zeroize(password);
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] hash = null;
            try {
                hash = pbkdf2(password, salt, ITERATIONS, KEY_LEN_BYTES);
            } catch (Exception e) {
                zeroize(password);
                return false;
            } finally {
                zeroize(password);
            }
            String record = "v1$" + ITERATIONS + "$" + b64(salt) + "$" + b64(hash);
            users.put(username, record);
            return true;
        }

        public boolean verify(String username, char[] password) {
            String record = users.get(username);
            if (record == null || password == null) {
                zeroize(password);
                return false;
            }
            try {
                String[] parts = record.split("\\$");
                if (parts.length != 4 || !parts[0].equals("v1")) {
                    zeroize(password);
                    return false;
                }
                int iterations = Integer.parseInt(parts[1]);
                byte[] salt = Base64.getDecoder().decode(parts[2]);
                byte[] expected = Base64.getDecoder().decode(parts[3]);
                byte[] actual = pbkdf2(password, salt, iterations, expected.length);
                return constantTimeEquals(expected, actual);
            } catch (Exception e) {
                return false;
            } finally {
                zeroize(password);
            }
        }

        public String getRecord(String username) {
            return users.get(username);
        }

        private static boolean validateUsername(String username) {
            if (username == null) return false;
            if (username.length() < 3 || username.length() > 32) return false;
            for (int i = 0; i < username.length(); i++) {
                char c = username.charAt(i);
                if (!(Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '-')) return false;
            }
            return true;
        }

        private static boolean validatePassword(char[] password) {
            if (password == null || password.length < 10) return false;
            boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false, hasSpace = false;
            for (char c : password) {
                if (Character.isUpperCase(c)) hasUpper = true;
                else if (Character.isLowerCase(c)) hasLower = true;
                else if (Character.isDigit(c)) hasDigit = true;
                else if (Character.isWhitespace(c)) hasSpace = true;
                else hasSpecial = true;
            }
            return !hasSpace && hasUpper && hasLower && hasDigit && hasSpecial;
        }

        private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen)
                throws NoSuchAlgorithmException, InvalidKeySpecException {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance(KDF_ALGO);
            try {
                return skf.generateSecret(spec).getEncoded();
            } finally {
                spec.clearPassword();
            }
        }

        private static boolean constantTimeEquals(byte[] a, byte[] b) {
            if (a == null || b == null) return false;
            if (a.length != b.length) return false;
            int result = 0;
            for (int i = 0; i < a.length; i++) {
                result |= a[i] ^ b[i];
            }
            return result == 0;
        }

        private static String b64(byte[] data) {
            return Base64.getEncoder().encodeToString(data);
        }

        private static void zeroize(char[] arr) {
            if (arr != null) {
                java.util.Arrays.fill(arr, '\0');
            }
        }
    }

    public static void main(String[] args) {
        SecureUserStore store = new SecureUserStore();

        boolean t1 = store.signup("alice", "Str0ngPass!".toCharArray());
        System.out.println("Test1 (valid signup): " + t1);

        boolean t2 = store.signup("alice", "An0ther#Pass".toCharArray());
        System.out.println("Test2 (duplicate username): " + t2);

        boolean t3 = store.signup("bob space", "Str0ngPass!".toCharArray());
        System.out.println("Test3 (invalid username): " + t3);

        boolean t4 = store.signup("carol", "weak".toCharArray());
        System.out.println("Test4 (weak password): " + t4);

        boolean t5 = store.signup("charlie", "G00d#Password".toCharArray());
        System.out.println("Test5 (another valid signup): " + t5);

        boolean verify1 = store.verify("alice", "Str0ngPass!".toCharArray());
        System.out.println("Verify alice: " + verify1);

        boolean verify2 = store.verify("alice", "WrongPass!".toCharArray());
        System.out.println("Verify alice wrong: " + verify2);
    }
}