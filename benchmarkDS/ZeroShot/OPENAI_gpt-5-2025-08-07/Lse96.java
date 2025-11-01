import java.security.SecureRandom;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse96 {

    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 210000;
    private static final int KEY_LEN = 32; // 256-bit
    private static final Map<String, String> DB = new HashMap<>();

    // Generate cryptographically secure random salt
    public static byte[] generateSalt(int length) {
        SecureRandom sr = new SecureRandom();
        byte[] salt = new byte[length];
        sr.nextBytes(salt);
        return salt;
    }

    // Hash password with PBKDF2-HMAC-SHA256
    public static byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLen) throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        try {
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    // Hex encoding
    public static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    // Hex decoding
    public static byte[] fromHex(String hex) {
        int len = hex.length();
        if (len % 2 != 0) throw new IllegalArgumentException("Invalid hex");
        byte[] out = new byte[len / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(hex.charAt(2 * i), 16);
            int lo = Character.digit(hex.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) throw new IllegalArgumentException("Invalid hex");
            out[i] = (byte) ((hi << 4) + lo);
        }
        return out;
    }

    // Constant-time comparison
    public static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= (a[i] ^ b[i]);
        }
        return result == 0;
    }

    // Register user: returns stored record (salt||hash) as hex concatenation
    public static String register(String username, char[] password) {
        if (username == null || username.isBlank()) {
            throw new IllegalArgumentException("Username required");
        }
        if (password == null || password.length < 8) {
            throw new IllegalArgumentException("Password must be at least 8 characters");
        }
        try {
            byte[] salt = generateSalt(SALT_LEN);
            byte[] hash = hashPassword(password, salt, ITERATIONS, KEY_LEN);
            String record = toHex(salt) + toHex(hash);
            synchronized (DB) {
                DB.put(username, record);
            }
            // zeroize
            Arrays.fill(password, '\0');
            Arrays.fill(hash, (byte) 0);
            Arrays.fill(salt, (byte) 0);
            return record;
        } catch (Exception e) {
            throw new RuntimeException("Registration failed", e);
        }
    }

    // Verify password against stored record
    public static boolean verify(String username, char[] password) {
        String record;
        synchronized (DB) {
            record = DB.get(username);
        }
        if (record == null) return false;
        try {
            String saltHex = record.substring(0, SALT_LEN * 2);
            String hashHex = record.substring(SALT_LEN * 2);
            byte[] salt = fromHex(saltHex);
            byte[] expectedHash = fromHex(hashHex);
            byte[] actualHash = hashPassword(password, salt, ITERATIONS, KEY_LEN);
            boolean ok = constantTimeEquals(expectedHash, actualHash);
            Arrays.fill(password, '\0');
            Arrays.fill(salt, (byte) 0);
            Arrays.fill(expectedHash, (byte) 0);
            Arrays.fill(actualHash, (byte) 0);
            return ok;
        } catch (Exception e) {
            return false;
        }
    }

    // For demonstration: returns the stored hex for a username
    public static String getStoredRecord(String username) {
        synchronized (DB) {
            return DB.get(username);
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] users = {"alice", "bob", "charlie", "dave", "eve"};
        String[] passwords = {"password1!", "S33cr3t!", "superlongpassword123", "Pa$$w0rd123", "Secure🔒123!"};

        for (int i = 0; i < users.length; i++) {
            String stored = register(users[i], passwords[i].toCharArray());
            System.out.println(users[i] + " -> " + stored);
        }

        // Verify tests
        System.out.println("Verify alice correct: " + verify("alice", "password1!".toCharArray()));
        System.out.println("Verify alice wrong: " + verify("alice", "wrongpass".toCharArray()));
        System.out.println("Verify bob correct: " + verify("bob", "S33cr3t!".toCharArray()));
        System.out.println("Stored for charlie: " + getStoredRecord("charlie"));
        System.out.println("Verify eve correct: " + verify("eve", "Secure🔒123!".toCharArray()));
    }
}