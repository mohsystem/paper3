import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;

public class Lse95 {
    // In-memory "database": username -> hex(hash) + hex(salt)
    private static final Map<String, String> DB = new ConcurrentHashMap<>();

    private static final int SALT_LEN = 16; // 16 bytes salt
    private static final int KEY_LEN = 32;  // 32 bytes (256-bit) derived key
    private static final int ITERATIONS = 210000; // PBKDF2 iterations

    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");

    public static boolean registerUser(String username, char[] password) {
        if (!isValidUsername(username) || !isStrongPassword(password)) {
            zeroize(password);
            return false;
        }
        if (DB.containsKey(username)) {
            zeroize(password);
            return false;
        }
        byte[] salt = new byte[SALT_LEN];
        SECURE_RANDOM.nextBytes(salt);

        byte[] derived = null;
        try {
            derived = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
            String stored = toHex(derived) + toHex(salt); // append salt to hash
            DB.put(username, stored);
            return true;
        } catch (Exception e) {
            return false;
        } finally {
            zeroize(password);
            zeroize(salt);
            zeroize(derived);
        }
    }

    public static boolean verifyLogin(String username, char[] password) {
        String stored = DB.get(username);
        if (stored == null) {
            zeroize(password);
            return false;
        }
        if (stored.length() < SALT_LEN * 2) {
            zeroize(password);
            return false;
        }
        String saltHex = stored.substring(stored.length() - SALT_LEN * 2);
        String hashHex = stored.substring(0, stored.length() - SALT_LEN * 2);
        byte[] salt = fromHex(saltHex);
        byte[] expectedHash = fromHex(hashHex);

        byte[] computed = null;
        try {
            computed = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
            return MessageDigest.isEqual(expectedHash, computed);
        } catch (Exception e) {
            return false;
        } finally {
            zeroize(password);
            zeroize(salt);
            zeroize(expectedHash);
            zeroize(computed);
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isStrongPassword(char[] password) {
        if (password == null || password.length < 12) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else if ("!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\ ".indexOf(c) >= 0) hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        javax.crypto.SecretKeyFactory skf = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(password, salt, iterations, keyLen * 8);
        try {
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    private static String toHex(byte[] data) {
        if (data == null) return "";
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
            sb.append(Character.forDigit((b & 0xF), 16));
        }
        return sb.toString();
    }

    private static byte[] fromHex(String hex) {
        if (hex == null || (hex.length() % 2) != 0) return new byte[0];
        int len = hex.length() / 2;
        byte[] out = new byte[len];
        for (int i = 0; i < len; i++) {
            int hi = Character.digit(hex.charAt(2 * i), 16);
            int lo = Character.digit(hex.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) return new byte[0];
            out[i] = (byte) ((hi << 4) + lo);
        }
        return out;
    }

    private static void zeroize(byte[] arr) {
        if (arr != null) Arrays.fill(arr, (byte) 0);
    }

    private static void zeroize(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    // Demonstration with 5 test cases
    public static void main(String[] args) {
        // 1) Successful registration
        System.out.println("Test1 register alice: " + registerUser("alice_01", "Str0ng!Passw0rd".toCharArray()));
        // 2) Successful login
        System.out.println("Test2 login alice correct: " + verifyLogin("alice_01", "Str0ng!Passw0rd".toCharArray()));
        // 3) Failed login due to wrong password
        System.out.println("Test3 login alice wrong: " + verifyLogin("alice_01", "WrongPassword!1".toCharArray()));
        // 4) Duplicate registration should fail
        System.out.println("Test4 duplicate register alice: " + registerUser("alice_01", "An0ther!Passw0rd".toCharArray()));
        // 5) Weak password should fail registration
        System.out.println("Test5 register weak bob: " + registerUser("bob", "weakpass".toCharArray()));
    }
}