import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {
    // Compiled once when the class is loaded
    private static final Pattern ALLOW_PATTERN = Pattern.compile(
            "^https://([A-Za-z0-9-]+\\.)*example\\.com(:[0-9]{1,5})?(/.*)?$",
            Pattern.CASE_INSENSITIVE
    );

    private static final int MAX_URL_LEN = 2048;
    private static final String SAFE_DEFAULT = "https://www.example.com/";

    // Returns targetUrl if it matches the allow-list regex; otherwise returns a safe default.
    public static String resolveRedirect(String targetUrl, String defaultUrl) {
        String safeFallback = (isAllowed(defaultUrl) ? defaultUrl : SAFE_DEFAULT);
        if (isAllowed(targetUrl)) {
            return targetUrl;
        }
        return safeFallback;
    }

    private static boolean isAllowed(String url) {
        if (url == null) return false;
        if (url.length() == 0 || url.length() > MAX_URL_LEN) return false;
        // Basic header injection defense for log/display; not constructing headers here, but be safe
        if (url.indexOf('\r') >= 0 || url.indexOf('\n') >= 0) return false;
        Matcher m = ALLOW_PATTERN.matcher(url);
        return m.matches();
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] tests = new String[][]{
            { "https://example.com/path",               "https://www.example.com/home" },
            { "https://sub.example.com/",               "https://www.example.com/fallback" },
            { "https://evil.com/",                      "https://www.example.com/fallback" },
            { "http://example.com/",                    "https://www.example.com/fallback" },
            { "https://example.com/" + "a".repeat(2100),"http://insecure.example.com/" } // overly long target; invalid default
        };

        for (int i = 0; i < tests.length; i++) {
            String target = tests[i][0];
            String def    = tests[i][1];
            String result = resolveRedirect(target, def);
            System.out.println("Test " + (i+1) + ":");
            System.out.println("  target = " + (target.length() <= 80 ? target : target.substring(0, 80) + "..."));
            System.out.println("  default= " + def);
            System.out.println("  result = " + result);
        }
    }
}