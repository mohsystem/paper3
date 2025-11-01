import java.util.regex.Pattern;
import java.util.Arrays;

public class Lse28 {
    // Compile the regular expression once when the class is loaded
    private static final Pattern ALLOWED_URL = Pattern.compile("^https?://(?:www\\.)?example\\.com(?:/.*)?$", Pattern.CASE_INSENSITIVE);

    // Function that uses the compiled regex to decide redirect target
    public static String redirect(String targetUrl, String defaultUrl) {
        if (targetUrl == null || defaultUrl == null) return defaultUrl;
        if (ALLOWED_URL.matcher(targetUrl).matches()) {
            return targetUrl;
        }
        return defaultUrl;
    }

    public static void main(String[] args) {
        String defaultUrl = "https://example.com/default";
        String[] tests = new String[] {
            "https://example.com/path?a=1",
            "http://www.example.com",
            "https://evil.com/phish",
            "ftp://example.com/file",
            "https://sub.example.com/page"
        };

        for (String t : tests) {
            String result = redirect(t, defaultUrl);
            System.out.println(t + " -> " + result);
        }
    }
}