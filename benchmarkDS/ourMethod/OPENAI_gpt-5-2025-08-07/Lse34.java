import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse34 {

    // User record holding salt, hash and iterations
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

    static final class SecureAuthDB {
        private final Map<String, UserRecord> db = new LinkedHashMap<>();
        private static final int SALT_LEN = 16;
        private static final int HASH_LEN = 32; // 256-bit
        private static final int ITERATIONS = 210_000;
        private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");

        private SecureRandom getSecureRandom() {
            try {
                return SecureRandom.getInstanceStrong();
            } catch (Exception e) {
                return new SecureRandom();
            }
        }

        private boolean validateUsername(String username) {
            if (username == null) return false;
            if (username.length() < 3 || username.length() > 32) return false;
            return USERNAME_PATTERN.matcher(username).matches();
        }

        private boolean passwordPolicy(String password) {
            if (password == null) return false;
            if (password.length() < 10 || password.length() > 128) return false;
            boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
            for (int i = 0; i < password.length(); i++) {
                char c = password.charAt(i);
                if (Character.isUpperCase(c)) hasUpper = true;
                else if (Character.isLowerCase(c)) hasLower = true;
                else if (Character.isDigit(c)) hasDigit = true;
                else hasSpecial = true;
            }
            return hasUpper && hasLower && hasDigit && hasSpecial;
        }

        private byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws GeneralSecurityException {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            try {
                return skf.generateSecret(spec).getEncoded();
            } finally {
                spec.clearPassword();
            }
        }

        public boolean registerUser(String username, String password) {
            if (!validateUsername(username)) return false;
            if (!passwordPolicy(password)) return false;
            if (db.containsKey(username)) return false;

            byte[] salt = new byte[SALT_LEN];
            getSecureRandom().nextBytes(salt);

            char[] pwChars = password.toCharArray();
            byte[] hash = null;
            try {
                hash = pbkdf2(pwChars, salt, ITERATIONS, HASH_LEN);
            } catch (GeneralSecurityException e) {
                Arrays.fill(pwChars, '\0');
                return false;
            }
            Arrays.fill(pwChars, '\0');

            db.put(username, new UserRecord(salt, hash, ITERATIONS));
            return true;
        }

        public boolean authenticate(String username, String password) {
            if (!validateUsername(username)) return false;
            UserRecord rec = db.get(username);
            if (rec == null) return false;

            char[] pwChars = (password == null ? new char[0] : password.toCharArray());
            byte[] computed = null;
            try {
                computed = pbkdf2(pwChars, rec.salt, rec.iterations, rec.hash.length);
            } catch (GeneralSecurityException e) {
                Arrays.fill(pwChars, '\0');
                return false;
            } finally {
                Arrays.fill(pwChars, '\0');
            }
            boolean eq = MessageDigest.isEqual(rec.hash, computed);
            Arrays.fill(computed, (byte) 0);
            return eq;
        }

        public String login(String username, String password) {
            boolean ok = authenticate(username, password);
            return ok ? "Redirect: /dashboard" : "Redirect: /login";
        }

        // Utility to stringify stored record for debug in a secure way (no secrets)
        public String debugUser(String username) {
            UserRecord rec = db.get(username);
            if (rec == null) return "no-user";
            return "User[" + username + "]: salt=" + Base64.getEncoder().encodeToString(Arrays.copyOf(rec.salt, Math.min(4, rec.salt.length))) + "..., iter=" + rec.iterations;
        }
    }

    public static void main(String[] args) {
        SecureAuthDB db = new SecureAuthDB();

        // Seed users with strong passwords
        System.out.println("Register alice: " + db.registerUser("alice", "Str0ng!Pass1"));
        System.out.println("Register bob: " + db.registerUser("bob", "B0b$Secure#2024"));
        System.out.println("Register charlie: " + db.registerUser("charlie", "Ch@rlie2025!!"));

        // 5 test cases
        String[][] tests = new String[][]{
                {"alice", "Str0ng!Pass1"},     // success
                {"alice", "wrongpassword!"},   // wrong password
                {"unknown", "AnyPassw0rd!"},   // unknown user
                {"bad user", "Xx1!invalid"},   // invalid username (space)
                {"bob", "B0b$Secure#2024"}     // success
        };

        for (int i = 0; i < tests.length; i++) {
            String u = tests[i][0];
            String p = tests[i][1];
            String result = db.login(u, p);
            System.out.println("Test " + (i + 1) + " login(" + u + ", [redacted]): " + result);
        }
    }
}