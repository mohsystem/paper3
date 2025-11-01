import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse37 {

    public static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    public static final class AuthService {
        private final Map<String, UserRecord> db = new HashMap<>();
        private final SecureRandom rng;

        private static final int MIN_PASSWORD_LEN = 8;
        private static final int MAX_PASSWORD_LEN = 128;
        private static final int MAX_USERNAME_LEN = 64;
        private static final int ITERATIONS = 210_000;
        private static final int KEY_LEN = 32; // 256-bit
        private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.@-]{1,64}$");

        public AuthService() {
            SecureRandom tmp;
            try {
                tmp = SecureRandom.getInstanceStrong();
            } catch (Exception ex) {
                tmp = new SecureRandom();
            }
            this.rng = tmp;
        }

        private boolean validateUsername(String username) {
            if (username == null) return false;
            if (username.length() < 1 || username.length() > MAX_USERNAME_LEN) return false;
            return USERNAME_PATTERN.matcher(username).matches();
        }

        private boolean validatePassword(char[] password) {
            if (password == null) return false;
            if (password.length < MIN_PASSWORD_LEN || password.length > MAX_PASSWORD_LEN) return false;
            boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
            String specials = "!@#$%^&*()-_=+[]{};:,.?/~";
            for (char c : password) {
                if (Character.isUpperCase(c)) hasUpper = true;
                else if (Character.isLowerCase(c)) hasLower = true;
                else if (Character.isDigit(c)) hasDigit = true;
                else if (specials.indexOf(c) >= 0) hasSpecial = true;
                // else allow other printable chars but do not count to complexity
            }
            return hasUpper && hasLower && hasDigit && hasSpecial;
        }

        private byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
            Arrays.fill(password, '\0'); // best-effort wipe caller-provided array copy if we passed a copy
            try {
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                return skf.generateSecret(spec).getEncoded();
            } finally {
                spec.clearPassword();
            }
        }

        private boolean constantTimeEquals(byte[] a, byte[] b) {
            return MessageDigest.isEqual(a, b);
        }

        public boolean registerUser(String username, char[] password) {
            if (!validateUsername(username) || !validatePassword(password)) {
                Arrays.fill(password, '\0');
                return false;
            }
            try {
                if (db.containsKey(username)) {
                    Arrays.fill(password, '\0');
                    return false;
                }
                byte[] salt = new byte[16];
                rng.nextBytes(salt);
                char[] pwCopy = Arrays.copyOf(password, password.length);
                byte[] hash = hashPassword(pwCopy, salt, ITERATIONS, KEY_LEN);
                Arrays.fill(password, '\0');
                db.put(username, new UserRecord(salt, hash, ITERATIONS));
                return true;
            } catch (Exception e) {
                Arrays.fill(password, '\0');
                return false;
            }
        }

        public String authenticate(String username, char[] password) {
            try {
                if (!validateUsername(username) || password == null || password.length < 1 || password.length > MAX_PASSWORD_LEN) {
                    Arrays.fill(password == null ? new char[0] : password, '\0');
                    return "cant_login";
                }
                UserRecord rec = db.get(username);
                if (rec == null || rec.salt == null || rec.hash == null || rec.iterations < 1) {
                    Arrays.fill(password, '\0');
                    return "cant_login";
                }
                char[] pwCopy = Arrays.copyOf(password, password.length);
                byte[] calc = hashPassword(pwCopy, rec.salt, rec.iterations, rec.hash.length);
                boolean ok = constantTimeEquals(calc, rec.hash);
                Arrays.fill(password, '\0');
                Arrays.fill(pwCopy, '\0');
                Arrays.fill(calc, (byte) 0);
                return ok ? "success" : "cant_login";
            } catch (Exception e) {
                if (password != null) Arrays.fill(password, '\0');
                return "internal_server_error";
            }
        }

        // Testing helper to simulate an internal error by corrupting a record
        public void corruptUserRecord(String username) {
            UserRecord rec = db.get(username);
            if (rec != null) {
                db.put(username, new UserRecord(null, rec.hash, rec.iterations)); // null salt will cause error handling
            }
        }
    }

    // Strong random password generator: ensures complexity
    public static char[] generateStrongPassword(SecureRandom rng, int length) {
        final String U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        final String L = "abcdefghijklmnopqrstuvwxyz";
        final String D = "0123456789";
        final String S = "!@#$%^&*()-_=+[]{};:,.?/~";
        final String ALL = U + L + D + S;
        if (length < 12) length = 12;

        char[] pwd = new char[length];
        pwd[0] = U.charAt(rng.nextInt(U.length()));
        pwd[1] = L.charAt(rng.nextInt(L.length()));
        pwd[2] = D.charAt(rng.nextInt(D.length()));
        pwd[3] = S.charAt(rng.nextInt(S.length()));
        for (int i = 4; i < length; i++) {
            pwd[i] = ALL.charAt(rng.nextInt(ALL.length()));
        }
        // shuffle
        for (int i = pwd.length - 1; i > 0; i--) {
            int j = rng.nextInt(i + 1);
            char t = pwd[i]; pwd[i] = pwd[j]; pwd[j] = t;
        }
        return pwd;
    }

    public static void main(String[] args) {
        AuthService auth = new AuthService();
        SecureRandom rng;
        try {
            rng = SecureRandom.getInstanceStrong();
        } catch (Exception ex) {
            rng = new SecureRandom();
        }

        String user1 = "alice@example.com";
        String user2 = "bob_user-1";

        char[] pass1 = generateStrongPassword(rng, 14);
        char[] pass2 = generateStrongPassword(rng, 16);

        boolean r1 = auth.registerUser(user1, Arrays.copyOf(pass1, pass1.length));
        boolean r2 = auth.registerUser(user2, Arrays.copyOf(pass2, pass2.length));

        // Test 1: correct login for user1
        String t1 = auth.authenticate(user1, Arrays.copyOf(pass1, pass1.length));
        // Test 2: wrong password for user1 (mutate first char)
        char[] wrong = Arrays.copyOf(pass1, pass1.length);
        wrong[0] = wrong[0] == 'A' ? 'B' : 'A';
        String t2 = auth.authenticate(user1, wrong);
        // Test 3: non-existent user
        String t3 = auth.authenticate("charlie", generateStrongPassword(rng, 12));
        // Test 4: invalid username format
        String t4 = auth.authenticate("invalid user name with spaces", generateStrongPassword(rng, 12));
        // Test 5: simulate internal server error by corrupting record
        auth.corruptUserRecord(user2);
        String t5 = auth.authenticate(user2, Arrays.copyOf(pass2, pass2.length));

        System.out.println("Test1: " + (r1 ? t1 : "internal_server_error"));
        System.out.println("Test2: " + t2);
        System.out.println("Test3: " + t3);
        System.out.println("Test4: " + t4);
        System.out.println("Test5: " + (r2 ? t5 : "internal_server_error"));

        Arrays.fill(pass1, '\0');
        Arrays.fill(pass2, '\0');
    }
}