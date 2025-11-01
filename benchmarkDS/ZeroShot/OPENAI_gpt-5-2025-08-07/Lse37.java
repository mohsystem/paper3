import java.util.*;
import java.nio.charset.StandardCharsets;

public class Lse37 {
    private static final Map<String, char[]> DB = new HashMap<>();
    private static final char[] DUMMY_SECRET = "dummy_password_for_timing".toCharArray();
    private static final int MAX_USERNAME_LEN = 64;
    private static final int MAX_PASSWORD_LEN = 128;

    static {
        DB.put("alice", "StrongPass!42".toCharArray());
        DB.put("bob", "s3cr3t".toCharArray());
        DB.put("charlie", "passw0rd".toCharArray());
        DB.put("diana", "Qwerty!234".toCharArray());
    }

    public static String handleLogin(String username, String password) {
        try {
            if (username == null || password == null) {
                return "cant_login";
            }
            if (username.equals("__simulate_error__")) {
                throw new RuntimeException("Simulated internal error");
            }
            if (username.length() == 0 || username.length() > MAX_USERNAME_LEN || password.length() > MAX_PASSWORD_LEN) {
                return "cant_login";
            }

            char[] inputPwd = password.toCharArray();
            try {
                char[] stored = DB.get(username);
                if (stored == null) {
                    // Mitigate user enumeration by performing a constant-time compare against a dummy secret
                    constantTimeEquals(inputPwd, DUMMY_SECRET);
                    return "cant_login";
                }
                boolean ok = constantTimeEquals(inputPwd, stored);
                return ok ? "success" : "cant_login";
            } finally {
                // Best-effort wipe of the password char array
                Arrays.fill(inputPwd, '\0');
            }
        } catch (Exception ex) {
            return "internal_server_error";
        }
    }

    private static boolean constantTimeEquals(char[] a, char[] b) {
        int maxLen = Math.max(a.length, b.length);
        int diff = a.length ^ b.length;
        for (int i = 0; i < maxLen; i++) {
            char ca = i < a.length ? a[i] : 0;
            char cb = i < b.length ? b[i] : 0;
            diff |= (ca ^ cb);
        }
        return diff == 0;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(handleLogin("alice", "StrongPass!42")); // success
        System.out.println(handleLogin("alice", "wrong")); // cant_login
        System.out.println(handleLogin("unknown", "whatever")); // cant_login
        System.out.println(handleLogin("", "somepass")); // cant_login
        System.out.println(handleLogin("__simulate_error__", "x")); // internal_server_error
    }
}