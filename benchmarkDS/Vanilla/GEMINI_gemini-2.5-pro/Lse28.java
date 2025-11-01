import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {
    // The regular expression is compiled once when the class is loaded.
    private static final Pattern URL_PATTERN = Pattern.compile("^https?://(www\\.)?example\\.com/.*$");
    private static final String DEFAULT_URL = "https://default-redirect.com";

    /**
     * Determines the redirect URL based on a pattern match.
     *
     * @param targetUrl The URL to check.
     * @return The targetUrl if it matches the pattern, otherwise the default URL.
     */
    public static String getRedirectUrl(String targetUrl) {
        if (targetUrl != null) {
            Matcher matcher = URL_PATTERN.matcher(targetUrl);
            // If the target URL matches, it is used as the redirect target.
            if (matcher.matches()) {
                return targetUrl;
            }
        }
        // If the target URL does not match, a default URL is used.
        return DEFAULT_URL;
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://www.example.com/path/to/resource", // Should match
            "http://example.com/another-page",          // Should match
            "https://www.google.com",                   // Should not match
            "https://subdomain.example.com/page",       // Should not match
            "ftp://example.com/file"                    // Should not match
        };

        System.out.println("Running Java Test Cases:");
        for (String url : testUrls) {
            String redirectUrl = getRedirectUrl(url);
            System.out.println("Input: " + url + " -> Redirect: " + redirectUrl);
        }
    }
}