import java.util.*;
import java.nio.charset.StandardCharsets;

public class Lse37 {

    private static final Map<String, String> DB = new HashMap<>();
    private static final String DUMMY_PASSWORD = "dummy_secret_password_123";
    static {
        // Demo in-memory "database" with plaintext for simplicity
        DB.put("alice", "Wonderland!42");
        DB.put("bob", "Secure*Pass9");
        DB.put("charlie", "S3cret#Key");
    }

    private static boolean isPrintableAscii(String s) {
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch < 32 || ch > 126) {
                return false;
            }
        }
        return true;
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        int max = Math.max(a.length, b.length);
        int diff = a.length ^ b.length;
        for (int i = 0; i < max; i++) {
            byte ba = i < a.length ? a[i] : 0;
            byte bb = i < b.length ? b[i] : 0;
            diff |= (ba ^ bb);
        }
        return diff == 0;
    }

    public static String login(String username, String password) {
        try {
            if (username == null || password == null) {
                return "internal_server_error";
            }
            if (username.length() < 1 || username.length() > 128 || password.length() < 1 || password.length() > 128) {
                return "internal_server_error";
            }
            if (!isPrintableAscii(username) || !isPrintableAscii(password)) {
                return "internal_server_error";
            }

            String stored = DB.get(username);
            String compareAgainst = (stored != null) ? stored : DUMMY_PASSWORD;

            boolean match = constantTimeEquals(
                    password.getBytes(StandardCharsets.UTF_8),
                    compareAgainst.getBytes(StandardCharsets.UTF_8)
            );

            if (stored != null && match) {
                return "success";
            } else {
                return "cant_login";
            }
        } catch (Exception e) {
            return "internal_server_error";
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(login("alice", "Wonderland!42")); // success
        System.out.println(login("bob", "Secure*Pass9")); // success
        System.out.println(login("alice", "NotIt")); // cant_login
        System.out.println(login("dave", "anything")); // cant_login
        System.out.println(login(null, "x")); // internal_server_error
    }
}