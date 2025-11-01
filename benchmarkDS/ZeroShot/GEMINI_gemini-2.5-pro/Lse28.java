import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {

    // The regular expression is compiled once when the class is loaded.
    // It whitelists URLs from a specific, safe domain.
    private static final Pattern SAFE_URL_PATTERN = Pattern.compile("^https://safe\\.example\\.com/([a-zA-Z0-9/\\-]+)$");
    private static final String DEFAULT_URL = "https://example.com/default";

    /**
     * Determines the redirect URL based on a whitelist pattern.
     * If the target URL matches the pre-compiled regular expression, it is returned.
     * Otherwise, a default URL is returned.
     *
     * @param targetUrl The URL to validate for redirection.
     * @return The targetUrl if it's valid, otherwise the default URL.
     */
    public static String getRedirectUrl(String targetUrl) {
        if (targetUrl == null) {
            return DEFAULT_URL;
        }
        Matcher matcher = SAFE_URL_PATTERN.matcher(targetUrl);
        // The regular expression is used to match the target URL.
        // It must match the entire string.
        if (matcher.matches()) {
            // If the target URL matches, it is used as the redirect target.
            // The regex is not used to construct the URL, preventing manipulation.
            return targetUrl;
        } else {
            // If the target URL does not match, a default URL is used.
            return DEFAULT_URL;
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            // Test Case 1: Valid URL
            "https://safe.example.com/valid-path",
            // Test Case 2: Valid URL with sub-path
            "https://safe.example.com/user/profile-123",
            // Test Case 3: Invalid domain
            "https://unsafe.example.com/path",
            // Test Case 4: Invalid characters (query parameters are not allowed by the regex)
            "https://safe.example.com/path?query=malicious",
            // Test Case 5: URL trying to bypass with different protocol or domain format
            "http://safe.example.com/path"
        };

        System.out.println("Java Test Cases:");
        for (String url : testUrls) {
            System.out.println("Input:  " + url);
            System.out.println("Output: " + getRedirectUrl(url));
            System.out.println("--------------------");
        }
    }
}