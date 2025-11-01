import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse56 {

    // Simple in-memory user model and store
    static final class User {
        final String username;
        String email;
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        boolean loggedIn;

        User(String username, String email, byte[] salt, byte[] hash, int iterations) {
            this.username = username;
            this.email = email;
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
            this.loggedIn = false;
        }
    }

    private static final Map<String, User> USERS = new HashMap<>();
    private static final SecureRandom RNG = new SecureRandom();
    private static final int SALT_LEN = 16;
    private static final int DERIVED_KEY_LEN = 32; // 256-bit
    private static final int PBKDF2_ITERATIONS = 210_000;

    // Register a user with a strong password
    public static boolean registerUser(String username, String email, char[] password) {
        if (!validateUsername(username) || !validateEmail(email)) {
            return false;
        }
        if (!isStrongPassword(password)) {
            return false;
        }
        if (USERS.containsKey(username)) {
            return false;
        }
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] hash = pbkdf2Hash(password, salt, PBKDF2_ITERATIONS, DERIVED_KEY_LEN);
        if (hash == null) {
            return false;
        }
        USERS.put(username, new User(username, email, salt, hash, PBKDF2_ITERATIONS));
        secureZero(password);
        return true;
    }

    // Login verifies password and sets loggedIn
    public static boolean login(String username, char[] password) {
        User u = USERS.get(username);
        if (u == null) {
            secureZero(password);
            return false;
        }
        boolean ok = verifyPassword(u, password);
        secureZero(password);
        if (ok) {
            u.loggedIn = true;
        }
        return ok;
    }

    // Change email requires: logged in, old email matches, new email valid and different, confirmPassword correct
    public static boolean changeEmail(String username, String oldEmail, String newEmail, char[] confirmPassword) {
        User u = USERS.get(username);
        if (u == null) {
            secureZero(confirmPassword);
            return false;
        }
        if (!u.loggedIn) {
            secureZero(confirmPassword);
            return false;
        }
        if (!validateEmail(oldEmail) || !validateEmail(newEmail)) {
            secureZero(confirmPassword);
            return false;
        }
        if (!u.email.equals(oldEmail)) {
            secureZero(confirmPassword);
            return false;
        }
        if (u.email.equals(newEmail)) {
            secureZero(confirmPassword);
            return false;
        }
        boolean ok = verifyPassword(u, confirmPassword);
        secureZero(confirmPassword);
        if (!ok) {
            return false;
        }
        u.email = newEmail;
        return true;
    }

    // Helpers

    private static boolean validateUsername(String username) {
        if (username == null) return false;
        String u = username.trim();
        if (u.length() < 3 || u.length() > 50) return false;
        for (int i = 0; i < u.length(); i++) {
            char c = u.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.')) {
                return false;
            }
        }
        return true;
    }

    private static boolean validateEmail(String email) {
        if (email == null) return false;
        String e = email.trim();
        if (e.length() < 6 || e.length() > 254) return false;
        if (e.contains(" ")) return false;
        int at = e.indexOf('@');
        if (at <= 0 || at != e.lastIndexOf('@')) return false;
        String local = e.substring(0, at);
        String domain = e.substring(at + 1);
        if (local.length() == 0 || domain.length() < 3) return false;
        if (!domain.contains(".")) return false;
        return true;
    }

    private static boolean isStrongPassword(char[] pw) {
        if (pw == null) return false;
        if (pw.length < 12 || pw.length > 128) return false;
        boolean hasU = false, hasL = false, hasD = false, hasS = false;
        for (char c : pw) {
            if (Character.isUpperCase(c)) hasU = true;
            else if (Character.isLowerCase(c)) hasL = true;
            else if (Character.isDigit(c)) hasD = true;
            else if ("!@#$%^&*()-_=+[]{};:,.?/\\|`~'\"<>".indexOf(c) >= 0) hasS = true;
            else if (Character.isWhitespace(c)) return false;
        }
        return hasU && hasL && hasD && hasS;
    }

    private static byte[] pbkdf2Hash(char[] password, byte[] salt, int iterations, int dkLen) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] out = skf.generateSecret(spec).getEncoded();
            spec.clearPassword();
            return out;
        } catch (Exception e) {
            return null;
        }
    }

    private static boolean verifyPassword(User u, char[] provided) {
        byte[] candidate = pbkdf2Hash(provided, u.salt, u.iterations, DERIVED_KEY_LEN);
        if (candidate == null) {
            return false;
        }
        boolean eq = constantTimeEquals(u.hash, candidate);
        Arrays.fill(candidate, (byte) 0);
        return eq;
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) {
            r |= (a[i] ^ b[i]);
        }
        return r == 0;
    }

    private static void secureZero(char[] buf) {
        if (buf == null) return;
        Arrays.fill(buf, '\0');
    }

    // Demo test driver with 5 test cases
    public static void main(String[] args) {
        String username = "alice";
        String initialEmail = "alice@example.com";
        char[] strongPassword = generateStrongRandomPassword(16);

        System.out.println("Register user: " + registerUser(username, initialEmail, Arrays.copyOf(strongPassword, strongPassword.length)));

        // Test 1: try change email without login
        boolean t1 = changeEmail(username, initialEmail, "alice_new@example.com", Arrays.copyOf(strongPassword, strongPassword.length));
        System.out.println("Test1 change without login (expect false): " + t1);

        // Test 2: login with wrong password
        char[] wrong = "WrongPassw0rd!".toCharArray();
        boolean t2 = login(username, wrong);
        System.out.println("Test2 login wrong password (expect false): " + t2);
        secureZero(wrong);

        // Test 3: login with correct password
        boolean t3 = login(username, Arrays.copyOf(strongPassword, strongPassword.length));
        System.out.println("Test3 login correct password (expect true): " + t3);

        // Test 4: attempt change to same email
        boolean t4 = changeEmail(username, initialEmail, initialEmail, Arrays.copyOf(strongPassword, strongPassword.length));
        System.out.println("Test4 change to same email (expect false): " + t4);

        // Test 5: successful change email
        String newEmail = "alice_secure@example.net";
        boolean t5 = changeEmail(username, initialEmail, newEmail, Arrays.copyOf(strongPassword, strongPassword.length));
        System.out.println("Test5 change email valid (expect true): " + t5);

        // Zeroize local password
        secureZero(strongPassword);
    }

    private static char[] generateStrongRandomPassword(int length) {
        if (length < 12) length = 12;
        final char[] lowers = "abcdefghijklmnopqrstuvwxyz".toCharArray();
        final char[] uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ".toCharArray();
        final char[] digits = "0123456789".toCharArray();
        final char[] symbols = "!@#$%^&*()-_=+[]{};:,.?/\\|`~'\"<>".toCharArray();
        final char[] all = (new String(lowers) + new String(uppers) + new String(digits) + new String(symbols)).toCharArray();
        char[] out = new char[length];

        // Ensure complexity
        out[0] = lowers[RNG.nextInt(lowers.length)];
        out[1] = uppers[RNG.nextInt(uppers.length)];
        out[2] = digits[RNG.nextInt(digits.length)];
        out[3] = symbols[RNG.nextInt(symbols.length)];
        for (int i = 4; i < length; i++) {
            out[i] = all[RNG.nextInt(all.length)];
        }
        // Shuffle
        for (int i = out.length - 1; i > 0; i--) {
            int j = RNG.nextInt(i + 1);
            char tmp = out[i];
            out[i] = out[j];
            out[j] = tmp;
        }
        return out;
    }
}