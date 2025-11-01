import java.nio.charset.StandardCharsets;
import java.net.URLDecoder;
import java.util.regex.Pattern;

public class Lse30 {

    // Regex: starts with one or more alphanumeric or hyphen characters followed by a literal period
    private static final Pattern SAFE_PREFIX = Pattern.compile("^[A-Za-z0-9-]+\\..*");
    private static final int MAX_QUERY_LEN = 4096;
    private static final int MAX_VALUE_LEN = 2048;

    public static String determineRedirect(String queryString) {
        if (queryString == null) {
            return "/";
        }
        if (queryString.length() > MAX_QUERY_LEN) {
            return "/";
        }
        String target = extractTarget(queryString);
        if (target == null) {
            return "/";
        }
        if (target.length() == 0 || target.length() > MAX_VALUE_LEN) {
            return "/";
        }
        if (!isSafeAscii(target)) {
            return "/";
        }
        if (SAFE_PREFIX.matcher(target).matches()) {
            return target;
        }
        return "/";
    }

    private static String extractTarget(String query) {
        // Parse query parameters safely and extract "target"
        // Accept both "?a=b&target=..." and "a=b&target=..." styles
        String[] pairs = query.split("&", 200); // bound splits
        for (String pair : pairs) {
            if (pair.startsWith("?")) {
                pair = pair.substring(1);
            }
            int idx = pair.indexOf('=');
            if (idx < 0) {
                continue;
            }
            String name = pair.substring(0, idx);
            String value = pair.substring(idx + 1);
            if ("target".equals(name)) {
                try {
                    String decoded = URLDecoder.decode(value, StandardCharsets.UTF_8.name());
                    return decoded;
                } catch (Exception e) {
                    return null; // fail closed
                }
            }
        }
        return null;
    }

    private static boolean isSafeAscii(String s) {
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch < 0x20 || ch == 0x7F) { // no control chars
                return false;
            }
            if (ch > 0x7E) { // restrict to ASCII
                return false;
            }
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "target=abc.example.com",                    // valid -> abc.example.com
            "target=abc-",                               // invalid -> /
            "other=1&target=-sub.example.org",          // valid -> -sub.example.org
            "x=1&target=foo.&y=2",                      // valid -> foo.
            "target=%0d%0ahttps://evil.com"             // contains newline after decode -> /
        };
        for (String q : tests) {
            String result = determineRedirect(q);
            System.out.println("Query: " + q + " -> Redirect: " + result);
        }
    }
}