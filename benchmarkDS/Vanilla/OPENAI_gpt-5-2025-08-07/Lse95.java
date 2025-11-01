import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse95 {

    private static final Map<String, String> DB = new HashMap<>();
    private static final int SALT_LEN = 16;

    public static boolean registerUser(String username, String password) {
        if (username == null || password == null) return false;
        synchronized (DB) {
            if (DB.containsKey(username)) return false;
            byte[] salt = generateSalt(SALT_LEN);
            String hashHex = fnv1a64Hex(password, salt);
            String saltHex = bytesToHex(salt);
            DB.put(username, hashHex + ":" + saltHex);
            return true;
        }
    }

    public static boolean loginUser(String username, String password) {
        if (username == null || password == null) return false;
        String stored;
        synchronized (DB) {
            stored = DB.get(username);
        }
        if (stored == null) return false;
        int sep = stored.indexOf(':');
        if (sep <= 0) return false;
        String storedHash = stored.substring(0, sep);
        String saltHex = stored.substring(sep + 1);
        byte[] salt = hexToBytes(saltHex);
        String calcHash = fnv1a64Hex(password, salt);
        return constantTimeEquals(storedHash, calcHash);
    }

    private static byte[] generateSalt(int len) {
        byte[] salt = new byte[len];
        new SecureRandom().nextBytes(salt);
        return salt;
    }

    private static String fnv1a64Hex(String password, byte[] salt) {
        long fnvOffset = 0xcbf29ce484222325L;
        long fnvPrime = 0x100000001b3L;
        long hash = fnvOffset;
        byte[] pb = password.getBytes(StandardCharsets.UTF_8);
        for (byte b : pb) {
            hash ^= (b & 0xff);
            hash *= fnvPrime;
            hash &= 0xffffffffffffffffL;
        }
        for (byte b : salt) {
            hash ^= (b & 0xff);
            hash *= fnvPrime;
            hash &= 0xffffffffffffffffL;
        }
        return String.format("%016x", hash);
    }

    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        if (a.length() != b.length()) return false;
        int diff = 0;
        for (int i = 0; i < a.length(); i++) {
            diff |= a.charAt(i) ^ b.charAt(i);
        }
        return diff == 0;
    }

    private static String bytesToHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02x", b & 0xff));
        }
        return sb.toString();
    }

    private static byte[] hexToBytes(String hex) {
        int len = hex.length();
        if (len % 2 != 0) return new byte[0];
        byte[] out = new byte[len / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(hex.charAt(2 * i), 16);
            int lo = Character.digit(hex.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) return new byte[0];
            out[i] = (byte) ((hi << 4) + lo);
        }
        return out;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Register alice/password123: " + registerUser("alice", "password123")); // true
        System.out.println("Login alice/password123: " + loginUser("alice", "password123"));       // true
        System.out.println("Login alice/wrong: " + loginUser("alice", "wrong"));                   // false
        System.out.println("Register alice/another: " + registerUser("alice", "another"));         // false
        System.out.println("Register bob/S3cret!: " + registerUser("bob", "S3cret!"));             // true
    }
}