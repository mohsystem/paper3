import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task136 {
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32;
    private static final int ITERATIONS = 210_000;
    private static final long PASSWORD_EXPIRATION_SECONDS = 90L * 24L * 60L * 60L; // 90 days
    private static final SecureRandom RNG = new SecureRandom();

    static final class User {
        final String username;
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        final long createdAtEpochSecond;

        User(String username, byte[] salt, byte[] hash, int iterations, long createdAtEpochSecond) {
            this.username = username;
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
            this.iterations = iterations;
            this.createdAtEpochSecond = createdAtEpochSecond;
        }
    }

    static final class UserStore {
        private final Map<String, User> users = new HashMap<>();

        public synchronized String registerUser(String username, String password) {
            String uErr = validateUsername(username);
            if (uErr != null) {
                return "ERROR: " + uErr;
            }
            if (users.containsKey(username)) {
                return "ERROR: username already exists";
            }
            String pErr = validatePasswordPolicy(username, password);
            if (pErr != null) {
                return "ERROR: " + pErr;
            }
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] hash;
            char[] pwdChars = password.toCharArray();
            try {
                hash = pbkdf2(pwdChars, salt, ITERATIONS, HASH_LEN);
            } catch (Exception e) {
                return "ERROR: internal error";
            } finally {
                Arrays.fill(pwdChars, '\0');
            }
            long now = Instant.now().getEpochSecond();
            User user = new User(username, salt, hash, ITERATIONS, now);
            users.put(username, user);
            return "OK";
        }

        public synchronized boolean authenticate(String username, String password) {
            User user = users.get(username);
            if (user == null) {
                // mitigate user enumeration by doing a fake hash
                simulateWork(password);
                return false;
            }
            if (isExpired(user)) {
                simulateWork(password);
                return false;
            }
            char[] pwdChars = password.toCharArray();
            try {
                byte[] test = pbkdf2(pwdChars, user.salt, user.iterations, user.hash.length);
                return MessageDigest.isEqual(test, user.hash);
            } catch (Exception e) {
                return false;
            } finally {
                Arrays.fill(pwdChars, '\0');
            }
        }

        private static boolean isExpired(User user) {
            long now = Instant.now().getEpochSecond();
            return now - user.createdAtEpochSecond > PASSWORD_EXPIRATION_SECONDS;
        }

        private static void simulateWork(String password) {
            // Derive a fake hash to equalize timing on unknown user or expired password
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            char[] pwdChars = password.toCharArray();
            try {
                pbkdf2(pwdChars, salt, ITERATIONS, HASH_LEN);
            } catch (Exception ignored) {
            } finally {
                Arrays.fill(pwdChars, '\0');
            }
        }
    }

    private static String validateUsername(String username) {
        if (username == null) return "username required";
        if (username.length() < 3 || username.length() > 32) return "username length must be 3-32";
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-')) {
                return "username may contain letters, digits, '_' or '-'";
            }
        }
        return null;
    }

    private static String validatePasswordPolicy(String username, String password) {
        if (password == null) return "password required";
        if (password.length() < 12 || password.length() > 128) return "password length must be 12-128";
        boolean hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
        for (int i = 0; i < password.length(); i++) {
            char c = password.charAt(i);
            if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        if (!(hasLower && hasUpper && hasDigit && hasSpecial)) {
            return "password must contain upper, lower, digit, and special character";
        }
        String lowerPass = password.toLowerCase();
        String lowerUser = username.toLowerCase();
        if (lowerPass.contains(lowerUser)) return "password must not contain the username";
        Set<String> blacklist = new HashSet<>(Arrays.asList(
                "password", "123456", "123456789", "qwerty", "letmein", "welcome", "admin", "iloveyou"
        ));
        if (blacklist.contains(lowerPass)) return "password too common";
        return null;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int length) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, length * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] out = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return out;
    }

    // Utility function to present debug-safe status (no secrets)
    private static String boolStr(boolean b) {
        return b ? "true" : "false";
    }

    public static void main(String[] args) {
        UserStore store = new UserStore();

        // Test 1: Register alice with strong password
        String t1 = store.registerUser("alice", "Str0ng!Passw0rd");
        System.out.println("Test1 register alice: " + t1);

        // Test 2: Authenticate alice with correct password
        boolean t2 = store.authenticate("alice", "Str0ng!Passw0rd");
        System.out.println("Test2 auth alice correct: " + boolStr(t2));

        // Test 3: Authenticate alice with wrong password
        boolean t3 = store.authenticate("alice", "WrongPass!123");
        System.out.println("Test3 auth alice wrong: " + boolStr(t3));

        // Test 4: Register bob with weak password
        String t4 = store.registerUser("bob", "password");
        System.out.println("Test4 register bob weak: " + t4);

        // Test 5: Register alice again (duplicate)
        String t5 = store.registerUser("alice", "An0ther$tr0ngOne");
        System.out.println("Test5 register alice duplicate: " + t5);
    }
}