import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;

public class Lse96 {

    public static String fnv1aHash(String input) {
        final long FNV_OFFSET_BASIS = 0xcbf29ce484222325L;
        final long FNV_PRIME = 0x100000001b3L;
        long hash = FNV_OFFSET_BASIS;
        byte[] data = input.getBytes(StandardCharsets.UTF_8);
        for (byte b : data) {
            hash ^= (b & 0xff);
            hash *= FNV_PRIME;
            hash &= 0xffffffffffffffffL;
        }
        return String.format("%016x", hash);
    }

    public static String generateSalt(int lengthBytes) {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[lengthBytes];
        random.nextBytes(salt);
        StringBuilder sb = new StringBuilder(lengthBytes * 2);
        for (byte b : salt) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static String registerUser(Map<String, String> db, String username, String password) {
        String hashed = fnv1aHash(password);
        String salt = generateSalt(16);
        String stored = salt + ":" + hashed;
        db.put(username, stored);
        return stored;
    }

    public static void main(String[] args) {
        Map<String, String> db = new HashMap<>();
        String[][] tests = new String[][]{
            {"alice", "password123"},
            {"bob", "S3cure!"},
            {"charlie", "letmein"},
            {"diana", "hunter2"},
            {"eve", "correcthorsebatterystaple"}
        };
        for (String[] t : tests) {
            String stored = registerUser(db, t[0], t[1]);
            System.out.println(t[0] + " -> " + stored);
        }
    }
}