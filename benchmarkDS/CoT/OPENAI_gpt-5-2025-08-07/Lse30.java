import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse30 {
    private static final Pattern SAFE_PREFIX = Pattern.compile("^[A-Za-z0-9-]+\\.");
    private static final int MAX_LEN = 2048;

    public static String decideRedirectFromQuery(String query) {
        if (query == null) return "/";
        String q = query.startsWith("?") ? query.substring(1) : query;
        String target = getParam(q, "target");
        if (target == null) return "/";

        // Basic normalization and safety checks
        target = target.trim();
        if (target.length() == 0 || target.length() > MAX_LEN) return "/";
        if (target.indexOf('\r') >= 0 || target.indexOf('\n') >= 0) return "/";

        if (SAFE_PREFIX.matcher(target).find()) {
            return target;
        }
        return "/";
    }

    private static String getParam(String query, String key) {
        String[] parts = query.split("&");
        for (String part : parts) {
            int eq = part.indexOf('=');
            String k, v;
            if (eq >= 0) {
                k = part.substring(0, eq);
                v = part.substring(eq + 1);
            } else {
                k = part;
                v = "";
            }
            try {
                k = URLDecoder.decode(k, StandardCharsets.UTF_8.name());
                if (!"target".equals(k)) continue;
                v = URLDecoder.decode(v, StandardCharsets.UTF_8.name());
                return v;
            } catch (Exception e) {
                // On decode error, treat as missing/invalid
                return null;
            }
        }
        return null;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "target=abc.",
            "target=abc.def",
            "foo=1&target=-hyphen.",
            "target=..evil",
            "foo=bar"
        };
        for (String t : tests) {
            System.out.println(decideRedirectFromQuery(t));
        }
    }
}