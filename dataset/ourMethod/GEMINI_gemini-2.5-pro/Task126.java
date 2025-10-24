import java.util.Map;
import java.util.HashMap;

public class Task126 {

    private static final Map<String, String> URL_MAP;
    private static final String DEFAULT_URL = "https://example.com/404";

    static {
        URL_MAP = new HashMap<>();
        URL_MAP.put("home", "https://example.com/home");
        URL_MAP.put("profile", "https://example.com/user/profile");
        URL_MAP.put("settings", "https://example.com/user/settings");
        URL_MAP.put("login", "https://example.com/auth/login");
    }

    /**
     * Returns a redirect URL based on a predefined safe list.
     * This prevents open redirect vulnerabilities by never using the input
     * to construct the URL.
     *
     * @param pageKey A string key representing the desired page.
     * @return A hardcoded, safe URL string.
     */
    public static String getRedirectUrl(String pageKey) {
        if (pageKey == null || pageKey.isEmpty()) {
            return DEFAULT_URL;
        }
        // Use getOrDefault to safely return a pre-approved URL or the default.
        return URL_MAP.getOrDefault(pageKey, DEFAULT_URL);
    }

    public static void main(String[] args) {
        // Test cases
        String[] testInputs = {
            "home",
            "settings",
            "dashboard", // Invalid key
            "",          // Empty key
            "//malicious-site.com" // Malicious input attempt
        };

        System.out.println("Running test cases for getRedirectUrl:");
        for (String input : testInputs) {
            System.out.printf("Input: \"%s\" -> Redirect URL: %s%n", input, getRedirectUrl(input));
        }

        // Test case for null input
        System.out.printf("Input: null -> Redirect URL: %s%n", getRedirectUrl(null));
    }
}