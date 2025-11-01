import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lse39 {

    // In-memory user store with expected password bytes (for demo purposes)
    private static final Map<String, byte[]> USER_STORE = new HashMap<>();
    // Dummy secret to mitigate user enumeration/timing side-channel
    private static final byte[] DUMMY_SECRET = "Dummy#Secret1".getBytes(StandardCharsets.UTF_8);

    static {
        USER_STORE.put("alice", "Str0ngP@ss!".getBytes(StandardCharsets.UTF_8));
        USER_STORE.put("admin", "S3cure#Admin".getBytes(StandardCharsets.UTF_8));
    }

    // Constant-time comparison
    private static boolean secureEquals(byte[] a, byte[] b) {
        int diff = a.length ^ b.length;
        int max = Math.max(a.length, b.length);
        for (int i = 0; i < max; i++) {
            int x = i < a.length ? a[i] & 0xff : 0;
            int y = i < b.length ? b[i] & 0xff : 0;
            diff |= (x ^ y);
        }
        return diff == 0;
    }

    // Login function: accepts inputs as parameters and returns result message
    public static String login(String username, char[] password) {
        if (username == null || username.trim().isEmpty() || password == null) {
            if (password != null) Arrays.fill(password, '\0');
            return "Invalid username or password.";
        }

        byte[] passBytes = null;
        try {
            // Convert char[] to bytes; clear sensitive data afterward
            passBytes = new String(password).getBytes(StandardCharsets.UTF_8);

            byte[] expected = USER_STORE.get(username);
            boolean userExists = expected != null;
            if (!userExists) {
                expected = DUMMY_SECRET;
            }

            boolean ok = secureEquals(passBytes, expected);
            if (ok && userExists) {
                return "SESSION: " + username;
            }
            return "Invalid username or password.";
        } finally {
            if (passBytes != null) Arrays.fill(passBytes, (byte) 0);
            Arrays.fill(password, '\0');
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(login("alice", "Str0ngP@ss!".toCharArray()));
        System.out.println(login("alice", "wrong".toCharArray()));
        System.out.println(login("admin", "S3cure#Admin".toCharArray()));
        System.out.println(login("eve", "anything".toCharArray()));
        System.out.println(login("", "S3cure#Admin".toCharArray()));
    }
}