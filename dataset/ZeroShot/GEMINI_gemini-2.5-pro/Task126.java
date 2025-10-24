import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Task126 {

    // A secure, allowlisted map of page keys to their full URLs.
    private static final Map<String, String> ALLOWED_REDIRECTS;
    private static final String DEFAULT_URL = "https://example.com/error";

    static {
        ALLOWED_REDIRECTS = new HashMap<>();
        ALLOWED_REDIRECTS.put("home", "https://example.com/home");
        ALLOWED_REDIRECTS.put("about", "https://example.com/about");
        ALLOWED_REDIRECTS.put("contact", "https://example.com/contact-us");
        ALLOWED_REDIRECTS.put("products", "https://example.com/products");
    }

    /**
     * Securely gets a redirect URL based on a predefined allowlist.
     * Prevents open redirect vulnerabilities by never using user input to construct the URL.
     *
     * @param pageKey The user-provided key for the desired page.
     * @return The corresponding URL from the allowlist, or a default URL if the key is not found.
     */
    public static String getRedirectUrl(String pageKey) {
        // Use Optional to handle potential null input gracefully
        return Optional.ofNullable(pageKey)
                .map(ALLOWED_REDIRECTS::get) // Look up the key in the allowlist
                .orElse(DEFAULT_URL);       // Return default URL if key is null or not found
    }

    public static void main(String[] args) {
        String[] testCases = {
            "home",          // Test case 1: Valid input
            "contact",       // Test case 2: Another valid input
            "dashboard",     // Test case 3: Invalid input
            "https://evil.com", // Test case 4: Malicious input attempt
            null             // Test case 5: Null input
        };

        System.out.println("Running Java Test Cases:");
        for (String testCase : testCases) {
            String redirectUrl = getRedirectUrl(testCase);
            System.out.printf("Input: '%s' -> Redirecting to: %s%n", testCase, redirectUrl);
        }
    }
}