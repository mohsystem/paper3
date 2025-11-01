import java.util.regex.Pattern;

public class Lse30 {

    // The regex matches a string that starts with one or more alphanumeric characters
    // or hyphens, followed by a period.
    // NOTE: This logic is vulnerable to CWE-601: Open Redirect.
    // A user can be redirected to any external site that matches this pattern (e.g., "evil-site.com").
    // A safer implementation would validate the target against a whitelist of allowed domains or paths.
    private static final Pattern TARGET_PATTERN = Pattern.compile("^[a-zA-Z0-9-]+\\..*");

    /**
     * Determines the redirect URL based on a target parameter.
     *
     * @param target The target parameter from the request.
     * @return The target URL if it's valid, otherwise the root path "/".
     */
    public static String getRedirectUrl(String target) {
        // Rule #1: Ensure all input is validated.
        // A null or empty target is considered invalid.
        if (target == null || target.isEmpty()) {
            return "/";
        }

        if (TARGET_PATTERN.matcher(target).matches()) {
            return target;
        } else {
            return "/";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "example.com",                  // Valid
            "sub-domain.net/path?query=1",  // Valid
            "nodot",                        // Invalid: no dot
            ".startwithdot.com",            // Invalid: starts with dot
            "",                             // Invalid: empty string
            null                            // Invalid: null
        };

        System.out.println("Running Java test cases...");
        for (int i = 0; i < testCases.length; i++) {
            String target = testCases[i];
            String redirectUrl = getRedirectUrl(target);
            System.out.printf("Test Case %d: target=\"%s\", redirect_to=\"%s\"\n", i + 1, target, redirectUrl);
        }
    }
}