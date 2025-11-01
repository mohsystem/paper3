import java.util.*;

public class Lse136 {
    private static final String CORRECT_USER = "admin";
    private static final String CORRECT_PASS = "secret";

    public static String check_mod(String url) {
        String[] pq = splitPathAndQuery(url);
        String path = pq[0];
        String query = pq[1];

        if (!"/check_mod".equals(path)) return "false";

        String username = null, password = null;
        if (query != null && !query.isEmpty()) {
            String[] pairs = query.split("&");
            for (String pair : pairs) {
                if (pair.isEmpty()) continue;
                int eq = pair.indexOf('=');
                String key, val;
                if (eq >= 0) {
                    key = urlDecode(pair.substring(0, eq));
                    val = urlDecode(pair.substring(eq + 1));
                } else {
                    key = urlDecode(pair);
                    val = "";
                }
                if ("username".equals(key)) username = val;
                else if ("password".equals(key)) password = val;
            }
        }
        if (CORRECT_USER.equals(username) && CORRECT_PASS.equals(password)) {
            return "true";
        }
        return "false";
    }

    private static String[] splitPathAndQuery(String url) {
        String path;
        String query = "";
        int qmark = url.indexOf('?');
        if (qmark >= 0) {
            query = url.substring(qmark + 1);
        }
        int schemeIdx = url.indexOf("://");
        if (schemeIdx >= 0) {
            int slash = url.indexOf('/', schemeIdx + 3);
            if (slash >= 0) {
                path = url.substring(slash, (qmark >= 0 ? qmark : url.length()));
            } else {
                path = "/";
            }
        } else {
            if (qmark >= 0) {
                path = url.substring(0, qmark);
            } else {
                path = url;
            }
        }
        return new String[]{path, query};
    }

    private static int hexVal(char c) {
        if ('0' <= c && c <= '9') return c - '0;
        if ('a' <= c && c <= 'f') return c - 'a' + 10;
        if ('A' <= c && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    private static String urlDecode(String s) {
        StringBuilder out = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '+') {
                out.append(' ');
            } else if (c == '%' && i + 2 < s.length()) {
                int hi = hexVal(s.charAt(i + 1));
                int lo = hexVal(s.charAt(i + 2));
                if (hi >= 0 && lo >= 0) {
                    out.append((char) ((hi << 4) + lo));
                    i += 2;
                } else {
                    out.append(c);
                }
            } else {
                out.append(c);
            }
        }
        return out.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "/check_mod?username=admin&password=secret",
            "/check_mod?username=admin&password=wrong",
            "/other?username=admin&password=secret",
            "http://example.com/check_mod?username=admin&password=sec%72et",
            "/check_mod?password=secret&username=admin"
        };
        for (String t : tests) {
            System.out.println(check_mod(t));
        }
    }
}