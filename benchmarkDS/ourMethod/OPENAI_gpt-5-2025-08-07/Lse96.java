import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse96 {

    private static final SecureRandom RNG = new SecureRandom();
    private static final Map<String, String> DATABASE = new ConcurrentHashMap<>();
    private static final int SALT_LEN = 16; // 128-bit salt
    private static final int DERIVED_KEY_LEN = 32; // 256-bit derived key
    private static final int PBKDF2_ITERATIONS = 210_000;

    // Username: 3..32 chars, alnum _.- only
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,32}$");

    // Password policy: min 12, at least one upper, lower, digit, special; no spaces
    private static final Pattern PW_UPPER = Pattern.compile(".*[A-Z].*");
    private static final Pattern PW_LOWER = Pattern.compile(".*[a-z].*");
    private static final Pattern PW_DIGIT = Pattern.compile(".*\\d.*");
    private static final Pattern PW_SPECIAL = Pattern.compile(".*[^A-Za-z0-9].*");
    private static final Pattern PW_SPACE = Pattern.compile(".*\\s.*");

    public static byte[] generateSalt(int length) {
        if (length < 16 || length > 64) {
            throw new IllegalArgumentException("Invalid salt length");
        }
        byte[] salt = new byte[length];
        RNG.nextBytes(salt);
        return salt;
    }

    public static byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        if (password == null || salt == null) throw new IllegalArgumentException("Null input");
        if (iterations < 210_000) throw new IllegalArgumentException("Iterations too low");
        if (keyLen < 16 || keyLen > 64) throw new IllegalArgumentException("Invalid key length");

        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] dk = skf.generateSecret(spec).getEncoded();
        spec.clearPassword(); // clear inside PBEKeySpec
        return dk;
    }

    // Store base64(salt||hash)
    public static String concatSaltAndHashB64(byte[] salt, byte[] hash) {
        byte[] combined = new byte[salt.length + hash.length];
        System.arraycopy(salt, 0, combined, 0, salt.length);
        System.arraycopy(hash, 0, combined, salt.length, hash.length);
        String b64 = Base64.getEncoder().encodeToString(combined);
        Arrays.fill(combined, (byte) 0);
        return b64;
    }

    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    public static boolean isStrongPassword(char[] password) {
        if (password == null) return false;
        String p = new String(password);
        if (p.length() < 12) return false;
        if (PW_SPACE.matcher(p).matches()) return false;
        if (!PW_UPPER.matcher(p).matches()) return false;
        if (!PW_LOWER.matcher(p).matches()) return false;
        if (!PW_DIGIT.matcher(p).matches()) return false;
        if (!PW_SPECIAL.matcher(p).matches()) return false;
        return true;
    }

    public static boolean registerUser(String username, char[] password) {
        if (!isValidUsername(username) || !isStrongPassword(password)) {
            if (password != null) Arrays.fill(password, '\0');
            return false;
        }
        byte[] salt = null;
        byte[] hash = null;
        try {
            salt = generateSalt(SALT_LEN);
            hash = hashPassword(password, salt, PBKDF2_ITERATIONS, DERIVED_KEY_LEN);
            String record = concatSaltAndHashB64(salt, hash);
            String prev = DATABASE.putIfAbsent(username, record);
            boolean success = (prev == null);
            return success;
        } catch (Exception e) {
            return false;
        } finally {
            if (password != null) Arrays.fill(password, '\0');
            if (hash != null) Arrays.fill(hash, (byte) 0);
            if (salt != null) Arrays.fill(salt, (byte) 0);
        }
    }

    public static boolean verifyLogin(String username, char[] password) {
        String record = DATABASE.get(username);
        if (record == null) {
            if (password != null) Arrays.fill(password, '\0');
            return false;
        }
        byte[] decoded = null;
        byte[] salt = null;
        byte[] storedHash = null;
        byte[] candidate = null;
        try {
            decoded = Base64.getDecoder().decode(record);
            if (decoded.length != SALT_LEN + DERIVED_KEY_LEN) return false;
            salt = Arrays.copyOfRange(decoded, 0, SALT_LEN);
            storedHash = Arrays.copyOfRange(decoded, SALT_LEN, decoded.length);
            candidate = hashPassword(password, salt, PBKDF2_ITERATIONS, DERIVED_KEY_LEN);
            boolean ok = MessageDigest.isEqual(storedHash, candidate);
            return ok;
        } catch (Exception e) {
            return false;
        } finally {
            if (password != null) Arrays.fill(password, '\0');
            if (decoded != null) Arrays.fill(decoded, (byte) 0);
            if (salt != null) Arrays.fill(salt, (byte) 0);
            if (storedHash != null) Arrays.fill(storedHash, (byte) 0);
            if (candidate != null) Arrays.fill(candidate, (byte) 0);
        }
    }

    public static void main(String[] args) {
        // Test 1: valid registration
        boolean t1 = registerUser("alice", "Str0ng!Passw0rd".toCharArray());
        System.out.println("Test1 register valid: " + t1);

        // Test 2: invalid username
        boolean t2 = registerUser("a", "Another$trong123".toCharArray());
        System.out.println("Test2 invalid username rejected: " + (!t2));

        // Test 3: weak password
        boolean t3 = registerUser("bob", "weakpass".toCharArray());
        System.out.println("Test3 weak password rejected: " + (!t3));

        // Test 4: duplicate username
        boolean t4a = registerUser("carol", "V3ry$trongPass!".toCharArray());
        boolean t4b = registerUser("carol", "Diff3rent$trong!".toCharArray());
        System.out.println("Test4 duplicate username rejected: " + (t4a && !t4b));

        // Test 5: same password yields different stored records due to unique salt
        boolean r1 = registerUser("dave", "SamePassw0rd!!".toCharArray());
        boolean r2 = registerUser("erin", "SamePassw0rd!!".toCharArray());
        boolean sameStored = false;
        if (r1 && r2) {
            String s1 = DATABASE.get("dave");
            String s2 = DATABASE.get("erin");
            sameStored = s1 != null && s1.equals(s2);
        }
        boolean v1 = verifyLogin("dave", "SamePassw0rd!!".toCharArray());
        boolean v2 = verifyLogin("erin", "SamePassw0rd!!".toCharArray());
        System.out.println("Test5 unique salts and verify: " + (r1 && r2 && !sameStored && v1 && v2));
    }
}