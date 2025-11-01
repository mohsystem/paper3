import java.util.regex.Pattern;
import java.util.Objects;

public class Lse28 {

    // 1) Problem understanding:
    // - Compile the regex once (module load).
    // - Validate target URL against regex.
    // - If matches, return target URL; otherwise return default URL.
    // - Regex not used to construct the redirect URL.

    // 2) Security requirements:
    // - Avoid using user input to build regex.
    // - Cap input length to mitigate potential ReDoS or resource misuse.
    // - Provide a safe default if provided default URL is null/blank.

    // 3) Secure coding generation:
    private static final int MAX_URL_LENGTH = 2048;
    private static final Pattern ALLOWED_URL_PATTERN = Pattern.compile(
        "^https://(www\\.)?example\\.com(/[A-Za-z0-9._~!$&'()*+,;=:@%/-]*)?(\\?[A-Za-z0-9._~!$&'()*+,;=:@%/?-]*)?(#[A-Za-z0-9._~!$&'()*+,;=:@%/?-]*)?$",
        Pattern.CASE_INSENSITIVE
    );

    public static String determineRedirect(String targetUrl, String defaultUrl) {
        // Normalize default URL
        String safeDefault = (defaultUrl != null && !defaultUrl.isBlank()) ? defaultUrl : "https://example.com/";

        if (targetUrl == null) {
            return safeDefault;
        }

        // Defensive length check
        if (targetUrl.length() > MAX_URL_LENGTH) {
            return safeDefault;
        }

        // Validate with precompiled regex
        if (ALLOWED_URL_PATTERN.matcher(targetUrl).matches()) {
            return targetUrl;
        }
        return safeDefault;
    }

    // 4) Code review and 5) Secure code output: finalize logic and add tests.
    public static void main(String[] args) {
        String defaultUrl = "https://safe.example.org/home";

        String[] tests = new String[] {
            "https://example.com/welcome",
            "https://www.example.com/path?item=42#section",
            "https://evil.com/phish",
            "http://example.com/insecure",
            ""
        };

        for (int i = 0; i < tests.length; i++) {
            String result = determineRedirect(tests[i], defaultUrl);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}