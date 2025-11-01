import java.util.regex.Pattern;

public class Lse28 {

    // The default URL to use if the target URL does not match the safe pattern.
    private static final String DEFAULT_URL = "https://default.example.com";

    // The regular expression pattern to validate safe URLs.
    // It is final and static, so it is compiled only once when the class is loaded.
    private static final String SAFE_URL_PATTERN_STRING = "^https://safe\\.example\\.com/.*$";
    private static final Pattern SAFE_URL_PATTERN = Pattern.compile(SAFE_URL_PATTERN_STRING);

    /**
     * Determines the redirect URL based on a whitelist pattern.
     *
     * @param targetUrl The URL to check.
     * @return The targetUrl if it matches the safe pattern, otherwise the default URL.
     */
    public static String getRedirectUrl(String targetUrl) {
        // Rule #1: Validate input. Check for null or empty strings.
        if (targetUrl == null || targetUrl.trim().isEmpty()) {
            return DEFAULT_URL;
        }

        // If the target URL matches the pre-compiled regex, it is considered safe.
        if (SAFE_URL_PATTERN.matcher(targetUrl).matches()) {
            return targetUrl;
        }

        // If the target URL does not match, return the default URL to prevent open redirection.
        return DEFAULT_URL;
    }

    public static void main(String[] args) {
        String[] testUrls = {
            // Test Case 1: Valid URL that should be allowed.
            "https://safe.example.com/user/profile",
            // Test Case 2: URL from an untrusted domain.
            "https://unsafe.example.com/user/profile",
            // Test Case 3: URL with an insecure protocol.
            "http://safe.example.com/user/profile",
            // Test Case 4: Empty input string.
            "",
            // Test Case 5: Null input.
            null
        };

        System.out.println("--- Java Test Cases ---");
        for (String url : testUrls) {
            String redirectUrl = getRedirectUrl(url);
            System.out.printf("Input:  \"%s\"%nOutput: \"%s\"%n%n", url, redirectUrl);
        }
    }
}