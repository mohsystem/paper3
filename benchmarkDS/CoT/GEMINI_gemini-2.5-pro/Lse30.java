import java.util.regex.Pattern;

public class Lse30 {

    /**
     * Determines the redirect URL based on a target parameter.
     * The regular expression matches a string that starts with one or more 
     * alphanumeric characters or hyphens, followed by a period.
     * This is a security measure to prevent open redirect vulnerabilities.
     *
     * @param target The target query parameter.
     * @return The target if it matches the safe pattern, otherwise "/" (the root of the site).
     */
    public static String getRedirectUrl(String target) {
        // The regex ensures the target starts like a domain name (e.g., "example.com")
        // and is not a scheme-relative URL (e.g., "//evil.com") or an absolute path (e.g., "/login").
        String safeRedirectPattern = "^[a-zA-Z0-9-]+\\..*";

        // Check for null or empty input, which is considered invalid.
        if (target == null || target.trim().isEmpty()) {
            return "/";
        }

        // If the target matches the safe pattern, it's considered safe for redirection.
        if (Pattern.matches(safeRedirectPattern, target)) {
            return target;
        }

        // Otherwise, redirect to the root to prevent a potential open redirect attack.
        return "/";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "example.com/home",          // Valid: Standard domain with path
            "sub-domain.net?query=1",    // Valid: Subdomain with query string
            "/local/path",               // Invalid: Does not start with a domain-like pattern
            "justaword",                 // Invalid: No period
            "@bad.com/path"              // Invalid: Starts with a disallowed character
        };

        System.out.println("Running Java Test Cases:");
        for (String testCase : testCases) {
            String result = getRedirectUrl(testCase);
            System.out.printf("Input: \"%s\", Redirect To: \"%s\"\n", testCase, result);
        }
    }
}