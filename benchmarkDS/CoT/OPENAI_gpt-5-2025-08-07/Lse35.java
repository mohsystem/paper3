import java.util.*;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse35 {

    private static final Map<String, String> DB = new HashMap<>();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{1,64}$");
    private static final long FNV_OFFSET_BASIS = 0xcbf29ce484222325L;
    private static final long FNV_PRIME = 0x100000001b3L;
    private static final int HASH_ITERATIONS = 10000;

    public static void addUser(String username, String password) {
        if (!isValidUsername(username)) return;
        String hashed = hashPassword(username, password);
        DB.put(username, hashed);
    }

    public static String login(String username, String password) {
        if (username == null || password == null) {
            return "redirect:/login";
        }
        if (!isValidUsername(username)) {
            return "redirect:/login";
        }
        String stored = DB.get(username);
        if (stored == null) {
            return "redirect:/login";
        }
        String providedHash = hashPassword(username, password);
        if (secureEquals(stored, providedHash)) {
            return "redirect:/home";
        } else {
            return "redirect:/login";
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static String hashPassword(String username, String password) {
        String salt = "s@lt-" + username;
        byte[] data = (salt + ":" + password).getBytes(StandardCharsets.UTF_8);
        long h = FNV_OFFSET_BASIS;
        for (int i = 0; i < HASH_ITERATIONS; i++) {
            h = fnv1a64(h, data);
            // Mix iteration counter
            h ^= (i & 0xFF);
            h *= FNV_PRIME;
            h ^= ((i >>> 8) & 0xFF);
            h *= FNV_PRIME;
            h ^= ((i >>> 16) & 0xFF);
            h *= FNV_PRIME;
            h ^= ((i >>> 24) & 0xFF);
            h *= FNV_PRIME;
        }
        return toFixedHex16(h);
    }

    private static long fnv1a64(long current, byte[] input) {
        long h = current;
        for (byte b : input) {
            h ^= (b & 0xFFL);
            h *= FNV_PRIME;
        }
        return h;
    }

    private static String toFixedHex16(long value) {
        char[] hex = new char[16];
        long v = value;
        for (int i = 15; i >= 0; i--) {
            int nibble = (int) (v & 0xF);
            hex[i] = (char) (nibble < 10 ? ('0' + nibble) : ('a' + nibble - 10));
            v >>>= 4;
        }
        return new String(hex);
    }

    private static boolean secureEquals(String a, String b) {
        if (a == null || b == null) return false;
        int lenA = a.length();
        int lenB = b.length();
        int max = Math.max(lenA, lenB);
        int diff = lenA ^ lenB;
        for (int i = 0; i < max; i++) {
            char ca = i < lenA ? a.charAt(i) : 0;
            char cb = i < lenB ? b.charAt(i) : 0;
            diff |= (ca ^ cb);
        }
        return diff == 0;
    }

    public static void main(String[] args) {
        addUser("alice", "Pa$$w0rd!");
        addUser("bob", "secret1234");
        addUser("charlie", "CorrectHorseBatteryStaple");

        // 5 test cases
        System.out.println("Test1: " + login("alice", "Pa$$w0rd!")); // home
        System.out.println("Test2: " + login("alice", "wrong")); // login
        System.out.println("Test3: " + login("dave", "anything")); // login
        System.out.println("Test4: " + login("eve; DROP TABLE users", "x")); // login (invalid username)
        System.out.println("Test5: " + login("bob", "secret1234")); // home
    }
}