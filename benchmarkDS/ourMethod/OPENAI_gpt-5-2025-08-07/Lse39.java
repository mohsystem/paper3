import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse39 {
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32; // 256-bit
    private static final int ITERATIONS = 210000;
    private static final SecureRandom RNG = new SecureRandom();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{3,64}$");
    private static final int MIN_PASSWORD_LEN = 8;
    private static final int MAX_PASSWORD_LEN = 128;

    private static class User {
        final String username;
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        User(String u, byte[] s, byte[] h, int it) {
            this.username = u;
            this.salt = s;
            this.hash = h;
            this.iterations = it;
        }
    }

    private static final Map<String, User> USERS = new HashMap<>();

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isValidPasswordInput(char[] password) {
        if (password == null) return false;
        int len = password.length;
        return len >= MIN_PASSWORD_LEN && len <= MAX_PASSWORD_LEN;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        KeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= (a[i] ^ b[i]);
        }
        return result == 0;
    }

    private static void secureZero(char[] arr) {
        if (arr != null) {
            Arrays.fill(arr, '\0');
        }
    }

    private static void secureZero(byte[] arr) {
        if (arr != null) {
            Arrays.fill(arr, (byte) 0);
        }
    }

    public static boolean createUser(String username, char[] password) {
        if (!isValidUsername(username) || !isValidPasswordInput(password)) {
            return false;
        }
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] hash = null;
        try {
            hash = pbkdf2(password, salt, ITERATIONS, HASH_LEN);
        } catch (Exception e) {
            secureZero(salt);
            return false;
        } finally {
            secureZero(password);
        }
        USERS.put(username, new User(username, salt, hash, ITERATIONS));
        return true;
    }

    public static boolean verifyLogin(String username, char[] password) {
        if (!isValidUsername(username) || !isValidPasswordInput(password)) {
            secureZero(password);
            return false;
        }
        User u = USERS.get(username);
        if (u == null) {
            secureZero(password);
            return false;
        }
        byte[] computed = null;
        try {
            computed = pbkdf2(password, u.salt, u.iterations, HASH_LEN);
            return constantTimeEquals(computed, u.hash);
        } catch (Exception e) {
            return false;
        } finally {
            secureZero(password);
            secureZero(computed);
        }
    }

    // Public API: login function as requested
    public static String login(String username, String password) {
        if (password == null) return "Invalid username or password.";
        char[] pw = password.toCharArray();
        boolean ok = verifyLogin(username, pw);
        if (ok) {
            return "Redirect:/session";
        }
        return "Invalid username or password.";
    }

    // Helper to generate a strong password for testing (not printed, not logged)
    private static String generateStrongPassword(int length) {
        final String upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        final String lower = "abcdefghijklmnopqrstuvwxyz";
        final String digits = "0123456789";
        final String special = "!@#$%^&*()-_=+[]{}:,.?/~";
        final String all = upper + lower + digits + special;

        StringBuilder sb = new StringBuilder(length);
        // ensure complexity
        sb.append(upper.charAt(RNG.nextInt(upper.length())));
        sb.append(lower.charAt(RNG.nextInt(lower.length())));
        sb.append(digits.charAt(RNG.nextInt(digits.length())));
        sb.append(special.charAt(RNG.nextInt(special.length())));
        for (int i = 4; i < length; i++) {
            sb.append(all.charAt(RNG.nextInt(all.length())));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Setup a test user with a strong, runtime-generated password
        String testUser = "alice";
        String correctPwd = generateStrongPassword(16);
        boolean created = createUser(testUser, correctPwd.toCharArray());
        if (!created) {
            System.out.println("User setup failed");
            return;
        }

        // 5 Test cases
        String[] usernames = new String[] {
            testUser,        // correct
            testUser,        // wrong password
            "bob",           // non-existent user
            "",              // invalid username
            "al"             // too short username
        };
        String[] passwords = new String[] {
            correctPwd,
            "WrongPass!123",
            "Anything123!",
            "short",
            "Password!234"
        };

        for (int i = 0; i < 5; i++) {
            String result = login(usernames[i], passwords[i]);
            System.out.println("Test " + (i+1) + ": " + result);
        }
        // Do not log or print secrets. Clear sensitive variable copies if needed.
        // Strings are immutable in Java; avoiding storing secrets in Strings is recommended in real apps.
    }
}