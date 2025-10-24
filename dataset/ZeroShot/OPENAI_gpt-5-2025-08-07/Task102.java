import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task102 {
    private static final Map<String, String> DATABASE = new ConcurrentHashMap<>();
    private static final int ITERATIONS = 210000;
    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 32; // 256-bit
    private static final SecureRandom RNG = new SecureRandom();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,64}$");

    public static boolean updateUserPassword(String username, char[] newPassword) {
        if (!isValidUsername(username) || !isValidPassword(newPassword)) {
            wipe(newPassword);
            return false;
        }
        String hash = hashPassword(newPassword);
        wipe(newPassword);
        if (hash == null) return false;
        DATABASE.put(username, hash);
        return true;
    }

    public static String getStoredHash(String username) {
        return DATABASE.get(username);
    }

    public static boolean verifyPassword(String username, char[] password) {
        String stored = DATABASE.get(username);
        if (stored == null) {
            wipe(password);
            return false;
        }
        boolean ok = verifyHash(stored, password);
        wipe(password);
        return ok;
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isValidPassword(char[] pwd) {
        if (pwd == null || pwd.length < 8) return false;
        // Basic complexity: at least 3 of 4 categories
        boolean lower = false, upper = false, digit = false, special = false;
        for (char c : pwd) {
            if (Character.isLowerCase(c)) lower = true;
            else if (Character.isUpperCase(c)) upper = true;
            else if (Character.isDigit(c)) digit = true;
            else special = true;
        }
        int categories = (lower ? 1 : 0) + (upper ? 1 : 0) + (digit ? 1 : 0) + (special ? 1 : 0);
        return categories >= 3;
    }

    private static void wipe(char[] a) {
        if (a != null) Arrays.fill(a, '\0');
    }

    private static void wipe(byte[] a) {
        if (a != null) Arrays.fill(a, (byte) 0);
    }

    private static String hashPassword(char[] password) {
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] dk = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
        if (dk == null) return null;
        String saltHex = toHex(salt);
        String hashHex = toHex(dk);
        wipe(dk);
        return "pbkdf2_sha256:" + ITERATIONS + ":" + saltHex + ":" + hashHex;
    }

    private static boolean verifyHash(String stored, char[] password) {
        try {
            String[] parts = stored.split(":", 4);
            if (parts.length != 4) return false;
            if (!"pbkdf2_sha256".equalsIgnoreCase(parts[0])) return false;
            int iters = Integer.parseInt(parts[1]);
            byte[] salt = fromHex(parts[2]);
            byte[] expected = fromHex(parts[3]);
            byte[] dk = pbkdf2(password, salt, iters, expected.length);
            boolean equal = constantTimeEquals(expected, dk);
            wipe(dk);
            wipe(expected);
            wipe(salt);
            return equal;
        } catch (Exception e) {
            return false;
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) {
            r |= a[i] ^ b[i];
        }
        return r == 0;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] res = skf.generateSecret(spec).getEncoded();
            return res;
        } catch (Exception e) {
            return null;
        }
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
            sb.append(Character.forDigit(b & 0xF, 16));
        }
        return sb.toString();
    }

    private static byte[] fromHex(String hex) {
        int len = hex.length();
        if (len % 2 != 0) throw new IllegalArgumentException("Invalid hex");
        byte[] out = new byte[len / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(hex.charAt(2 * i), 16);
            int lo = Character.digit(hex.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) throw new IllegalArgumentException("Invalid hex");
            out[i] = (byte) ((hi << 4) | lo);
        }
        return out;
    }

    private static char[] toCharArray(String s) {
        CharBuffer cb = CharBuffer.wrap(s);
        ByteBuffer bb = StandardCharsets.UTF_8.encode(cb);
        char[] ca = new char[s.length()];
        s.getChars(0, s.length(), ca, 0);
        Arrays.fill(bb.array(), (byte) 0);
        return ca;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] users = {"alice", "bob_123", "charlie", "invalid user", "dave"};
        String[] passwords = {
            "CorrectHorseBatteryStaple!",
            "s3cureP@ssw0rd",
            "anotherStrong#Pass1",
            "weak pass",
            "short7!"
        };

        for (int i = 0; i < users.length; i++) {
            char[] pwd = toCharArray(passwords[i]);
            boolean ok = updateUserPassword(users[i], pwd);
            System.out.println("Update " + users[i] + ": " + ok);
        }

        // Verify some
        System.out.println("Stored hash alice present: " + (getStoredHash("alice") != null));
        System.out.println("Verify alice correct: " + verifyPassword("alice", toCharArray("CorrectHorseBatteryStaple!")));
        System.out.println("Verify alice wrong: " + verifyPassword("alice", toCharArray("WrongPassword!")));
        System.out.println("Stored hash invalid user present: " + (getStoredHash("invalid user") != null));
    }
}