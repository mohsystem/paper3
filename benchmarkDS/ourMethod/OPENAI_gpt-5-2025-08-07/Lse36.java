import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.time.Duration;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse36 {

    // Security parameters
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32; // 256-bit
    private static final int PBKDF2_ITERATIONS = 210_000;
    private static final Duration PASSWORD_MAX_AGE = Duration.ofDays(90);
    private static final String REDIRECT_URL = "https://example.com/home";
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,64}$");
    private static final int MAX_PASSWORD_LEN = 256;

    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    static final class User {
        final String username;
        final byte[] salt;          // unique per user
        final byte[] passwordHash;  // PBKDF2-HMAC-SHA256
        final int iterations;
        Instant createdAt;

        User(String username, byte[] salt, byte[] passwordHash, int iterations, Instant createdAt) {
            this.username = username;
            this.salt = salt;
            this.passwordHash = passwordHash;
            this.iterations = iterations;
            this.createdAt = createdAt;
        }
    }

    // In-memory "database"
    static final class InMemoryDb {
        private final Map<String, User> users = new ConcurrentHashMap<>();

        User get(String username) {
            return users.get(username);
        }

        boolean put(User u) {
            return users.putIfAbsent(u.username, u) == null;
        }
    }

    // Validate username format
    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    // Strong password policy: length >= 12, contains upper, lower, digit, special
    public static boolean isStrongPassword(char[] password) {
        if (password == null || password.length < 12 || password.length > MAX_PASSWORD_LEN) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else if ("!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~\\ ".indexOf(c) >= 0) hasSpecial = true;
            if (hasUpper && hasLower && hasDigit && hasSpecial) return true;
        }
        return false;
    }

    // PBKDF2-HMAC-SHA256
    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static byte[] randomBytes(int len) {
        byte[] out = new byte[len];
        SECURE_RANDOM.nextBytes(out);
        return out;
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) return false;
        int diff = 0;
        for (int i = 0; i < a.length; i++) {
            diff |= (a[i] ^ b[i]);
        }
        return diff == 0;
    }

    // Create user with hashed password
    public static boolean createUser(InMemoryDb db, String username, char[] password) {
        if (!isValidUsername(username)) return false;
        if (!isStrongPassword(password)) return false;
        byte[] salt = randomBytes(SALT_LEN);
        byte[] hash = null;
        try {
            hash = pbkdf2(password, salt, PBKDF2_ITERATIONS, HASH_LEN);
        } catch (GeneralSecurityException e) {
            Arrays.fill(salt, (byte) 0);
            return false;
        } finally {
            Arrays.fill(password, '\0'); // clear provided password
        }
        Instant now = Instant.now();
        User user = new User(username, salt, hash, PBKDF2_ITERATIONS, now);
        boolean ok = db.put(user);
        if (!ok) {
            Arrays.fill(salt, (byte) 0);
            Arrays.fill(hash, (byte) 0);
        }
        return ok;
    }

    // Login function: returns redirect URL on success or null on failure
    public static String login(InMemoryDb db, String username, char[] password) {
        if (!isValidUsername(username)) {
            Arrays.fill(password, '\0');
            return null;
        }
        if (password == null || password.length == 0 || password.length > MAX_PASSWORD_LEN) {
            if (password != null) Arrays.fill(password, '\0');
            return null;
        }
        User user = db.get(username);
        if (user == null) {
            Arrays.fill(password, '\0');
            return null;
        }
        byte[] computed = null;
        try {
            computed = pbkdf2(password, user.salt, user.iterations, HASH_LEN);
        } catch (GeneralSecurityException e) {
            Arrays.fill(password, '\0');
            return null;
        } finally {
            Arrays.fill(password, '\0');
        }
        try {
            boolean matches = constantTimeEquals(computed, user.passwordHash);
            boolean notExpired = Instant.now().isBefore(user.createdAt.plus(PASSWORD_MAX_AGE));
            if (matches && notExpired) {
                return REDIRECT_URL;
            } else {
                return null;
            }
        } finally {
            if (computed != null) Arrays.fill(computed, (byte) 0);
        }
    }

    // Utility: generate a strong random password that meets policy
    private static char[] generateStrongPassword(int length) {
        if (length < 12) length = 12;
        String upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        String lower = "abcdefghijklmnopqrstuvwxyz";
        String digits = "0123456789";
        String special = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~\\";
        String all = upper + lower + digits + special;
        StringBuilder sb = new StringBuilder(length);
        // Ensure at least one from each category
        sb.append(upper.charAt(SECURE_RANDOM.nextInt(upper.length())));
        sb.append(lower.charAt(SECURE_RANDOM.nextInt(lower.length())));
        sb.append(digits.charAt(SECURE_RANDOM.nextInt(digits.length())));
        sb.append(special.charAt(SECURE_RANDOM.nextInt(special.length())));
        for (int i = 4; i < length; i++) {
            sb.append(all.charAt(SECURE_RANDOM.nextInt(all.length())));
        }
        // Shuffle
        char[] chars = sb.toString().toCharArray();
        for (int i = chars.length - 1; i > 0; i--) {
            int j = SECURE_RANDOM.nextInt(i + 1);
            char tmp = chars[i];
            chars[i] = chars[j];
            chars[j] = tmp;
        }
        return chars;
    }

    // Test harness with 5 cases
    public static void main(String[] args) {
        InMemoryDb db = new InMemoryDb();

        // Test 1: Create user and successful login
        String user1 = "alice_1";
        char[] pass1 = generateStrongPassword(16);
        boolean created1 = createUser(db, user1, Arrays.copyOf(pass1, pass1.length));
        String res1 = login(db, user1, Arrays.copyOf(pass1, pass1.length));
        Arrays.fill(pass1, '\0');
        System.out.println("Test1_created=" + created1 + " loginSuccess=" + (REDIRECT_URL.equals(res1)));

        // Test 2: Wrong password
        String res2 = login(db, user1, "WrongPass123!".toCharArray());
        System.out.println("Test2_wrongPassword=" + (res2 == null));

        // Test 3: Non-existent user
        String res3 = login(db, "nonexistent", "SomeStrong!2345".toCharArray());
        System.out.println("Test3_nonexistent=" + (res3 == null));

        // Test 4: Invalid username format
        String res4 = login(db, "../baduser", "ValidPass!2345".toCharArray());
        System.out.println("Test4_invalidUsername=" + (res4 == null));

        // Test 5: Expired password
        String user2 = "bob-2";
        char[] pass2 = generateStrongPassword(18);
        boolean created2 = createUser(db, user2, Arrays.copyOf(pass2, pass2.length));
        // Force expiry by setting createdAt in the past beyond max age
        User bob = db.get(user2);
        if (bob != null) {
            bob.createdAt = Instant.now().minus(PASSWORD_MAX_AGE).minus(Duration.ofDays(1));
        }
        String res5 = login(db, user2, Arrays.copyOf(pass2, pass2.length));
        Arrays.fill(pass2, '\0');
        System.out.println("Test5_expiredPassword_userCreated=" + created2 + " loginBlocked=" + (res5 == null));

        // Example: print one hashed user record safely (no secrets)
        if (db.get(user1) != null) {
            User u = db.get(user1);
            System.out.println("User1_salt_b64=" + Base64.getEncoder().encodeToString(u.salt));
        }
    }
}