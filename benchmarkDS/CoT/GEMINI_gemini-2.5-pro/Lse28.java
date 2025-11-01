import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {

    // The regular expression is compiled once when the class is loaded.
    // This regex validates that the URL is on the example.com domain.
    private static final String ALLOWED_URL_PATTERN = "^https?://(www\\.)?example\\.com/.*$";
    private static final Pattern COMPILED_PATTERN = Pattern.compile(ALLOWED_URL_PATTERN);

    /**
     * Determines the redirect URL based on a whitelist pattern.
     * If the target URL matches the pre-compiled pattern, it is returned.
     * Otherwise, a default URL is returned.
     *
     * @param targetUrl The URL to validate.
     * @param defaultUrl The fallback URL if validation fails.
     * @return The validated targetUrl or the defaultUrl.
     */
    public static String getRedirectUrl(String targetUrl, String defaultUrl) {
        if (targetUrl == null || targetUrl.isEmpty()) {
            return defaultUrl;
        }

        Matcher matcher = COMPILED_PATTERN.matcher(targetUrl);

        // If the target URL matches the allowed pattern, it is used.
        if (matcher.matches()) {
            return targetUrl;
        } else {
            // Otherwise, the default URL is used.
            return defaultUrl;
        }
    }

    public static void main(String[] args) {
        String defaultUrl = "https://example.com/default";

        // Test Cases
        String[] testUrls = {
            "https://example.com/some/path", // Case 1: Valid HTTPS URL
            "http://www.example.com/another/page.html", // Case 2: Valid HTTP URL with www
            "https://evil.com/some/path", // Case 3: Invalid domain
            "https://example.com.evil.com/path", // Case 4: Invalid domain (tricky)
            "not a url" // Case 5: Invalid format
        };

        System.out.println("Running Java Test Cases:");
        for (int i = 0; i < testUrls.length; i++) {
            String target = testUrls[i];
            String redirect = getRedirectUrl(target, defaultUrl);
            System.out.printf("Test Case %d:\n", i + 1);
            System.out.printf("  Target:   %s\n", target);
            System.out.printf("  Redirect: %s\n\n", redirect);
        }
    }
}