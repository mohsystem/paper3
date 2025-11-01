import java.nio.charset.StandardCharsets;
import java.util.*;

public class Lse137 {

    private static final int MAX_VALUE_LEN = 256;

    private static class Account {
        final String username;
        final String password;
        final String session;
        Account(String u, String p, String s) {
            this.username = u;
            this.password = p;
            this.session = s;
        }
    }

    private static final List<Account> ACCOUNTS = Collections.unmodifiableList(Arrays.asList(
        new Account("mod_alice", "Str0ngP@ss!", "sess-AL1CE-123"),
        new Account("mod_bob", "B0b$Secure", "sess-BOB-456")
    ));

    public static boolean check_mod(String url) {
        if (url == null) return false;
        // Ensure path is /check_mod
        int qIdx = url.indexOf('?');
        String path = qIdx >= 0 ? url.substring(0, qIdx) : url;
        if (!"/check_mod".equals(path)) {
            return false;
        }
        Map<String, String> params = parseQuery(url);
        String username = safeGet(params, "username");
        String password = safeGet(params, "password");
        String session = safeGet(params, "session");
        if (username == null || password == null || session == null) return false;
        return auth_mod(username, password, session);
    }

    private static String safeGet(Map<String, String> map, String key) {
        if (map == null) return null;
        String v = map.get(key);
        if (v == null) return null;
        if (v.length() > MAX_VALUE_LEN) return null;
        return v;
    }

    public static boolean auth_mod(String username, String password, String session) {
        // Constant-time aggregate check over all accounts
        int match = 0;
        for (Account a : ACCOUNTS) {
            int ueq = ctEquals(username, a.username) ? 1 : 0;
            int peq = ctEquals(password, a.password) ? 1 : 0;
            int seq = ctEquals(session, a.session) ? 1 : 0;
            int all = ueq & peq & seq;
            match |= all;
        }
        return match == 1;
    }

    private static boolean ctEquals(String a, String b) {
        if (a == null) a = "";
        if (b == null) b = "";
        byte[] x = a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b.getBytes(StandardCharsets.UTF_8);
        int lx = x.length;
        int ly = y.length;
        int max = Math.max(lx, ly);
        int diff = lx ^ ly;
        for (int i = 0; i < max; i++) {
            byte xb = i < lx ? x[i] : 0;
            byte yb = i < ly ? y[i] : 0;
            diff |= (xb ^ yb);
        }
        return diff == 0;
    }

    private static Map<String, String> parseQuery(String url) {
        Map<String, String> map = new HashMap<>();
        int idx = url.indexOf('?');
        if (idx < 0 || idx == url.length() - 1) {
            return map;
        }
        String qs = url.substring(idx + 1);
        String[] pairs = qs.split("&");
        for (String pair : pairs) {
            if (pair.isEmpty()) continue;
            int eq = pair.indexOf('=');
            String key = eq >= 0 ? pair.substring(0, eq) : pair;
            String val = eq >= 0 ? pair.substring(eq + 1) : "";
            key = urlDecodeSafe(key);
            val = urlDecodeSafe(val);
            if (key == null || val == null) continue;
            if (key.length() == 0) continue;
            if (val.length() > MAX_VALUE_LEN) continue;
            if (key.equals("username") || key.equals("password") || key.equals("session")) {
                map.put(key, val);
            }
        }
        return map;
    }

    private static String urlDecodeSafe(String s) {
        if (s == null) return null;
        int len = s.length();
        StringBuilder out = new StringBuilder(Math.min(len, MAX_VALUE_LEN));
        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            if (c == '+') {
                if (out.length() >= MAX_VALUE_LEN) return null;
                out.append(' ');
            } else if (c == '%' && i + 2 < len) {
                int hi = hexVal(s.charAt(i + 1));
                int lo = hexVal(s.charAt(i + 2));
                if (hi < 0 || lo < 0) {
                    return null; // invalid percent-encoding
                }
                char decoded = (char) ((hi << 4) | lo);
                if (out.length() >= MAX_VALUE_LEN) return null;
                out.append(decoded);
                i += 2;
            } else {
                if (out.length() >= MAX_VALUE_LEN) return null;
                out.append(c);
            }
        }
        return out.toString();
    }

    private static int hexVal(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "/check_mod?username=mod_alice&password=Str0ngP%40ss%21&session=sess-AL1CE-123",
            "/check_mod?username=mod_alice&password=wrong&session=sess-AL1CE-123",
            "/check_mod?username=mod_bob&password=B0b%24Secure&session=bad-session",
            "/check_mod?username=joe&password=anything&session=whatever",
            "/check_mod?username=mod_alice&password=Str0ngP%40ss%21" // missing session
        };
        for (int i = 0; i < tests.length; i++) {
            boolean res = check_mod(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}