import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse30 {
    /**
     * Determines the redirect URL based on a regex pattern.
     * WARNING: The logic described in the prompt is vulnerable to Open Redirect attacks.
     * A user can be redirected to any external domain that matches the pattern (e.g., "evil.com").
     * A more secure implementation should validate the domain against a strict allow-list.
     *
     * @param target The target URL, supposedly from a query parameter.
     * @return The URL to redirect to (either the target or "/").
     */
    public static String getRedirectUrl(String target) {
        if (target == null || target.trim().isEmpty()) {
            System.out.println("Target is null or empty. Redirecting to root.");
            return "/";
        }
        // Regex: matches a string that starts with one or more alphanumeric characters or hyphens,
        // followed by a period.
        String regex = "^[a-zA-Z0-9-]+\\..*";
        Pattern pattern = Pattern.compile(regex);
        Matcher matcher = pattern.matcher(target);

        if (matcher.matches()) {
            System.out.println("Target '" + target + "' matches the pattern. Redirecting to target.");
            return target;
        } else {
            System.out.println("Target '" + target + "' does not match the pattern. Redirecting to root.");
            return "/";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "dashboard.myapp.com/user/1", // Valid
            "some-service.net/api/v1",    // Valid
            "example.com",                // Valid
            "/settings/profile",          // Invalid
            "malicious.com"               // Valid (demonstrates open redirect risk)
        };

        System.out.println("--- Java Test Cases ---");
        for (String testCase : testCases) {
            String redirectUrl = getRedirectUrl(testCase);
            System.out.println("=> Final Redirect URL: " + redirectUrl);
            System.out.println("-------------------------");
        }
    }
}