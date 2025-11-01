
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {
    // Compile regex once at class loading time
    // Using a simple, bounded pattern to prevent ReDoS
    private static final Pattern URL_PATTERN = Pattern.compile(
        "^https://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/[a-zA-Z0-9-._~:/?#\\\\[\\\\]@!$&'()*+,;=]*)?$"
    );
    
    private static final String DEFAULT_URL = "https://www.example.com/default";
    
    /**
     * Validates and returns redirect URL based on pattern matching
     * @param targetUrl The URL to validate
     * @return Valid redirect URL or default URL
     */
    public static String getRedirectUrl(String targetUrl) {
        // Null/empty check
        if (targetUrl == null || targetUrl.trim().isEmpty()) {
            return DEFAULT_URL;
        }
        
        // Trim and validate length to prevent DoS
        targetUrl = targetUrl.trim();
        if (targetUrl.length() > 2048) {
            return DEFAULT_URL;
        }
        
        // Match against compiled pattern
        Matcher matcher = URL_PATTERN.matcher(targetUrl);
        
        // If matches, return the original URL (not constructed from regex)
        if (matcher.matches()) {
            return targetUrl;
        }
        
        // Default fallback
        return DEFAULT_URL;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + getRedirectUrl("https://www.example.com/page"));
        System.out.println("Test 2: " + getRedirectUrl("https://sub.example.com/resource"));
        System.out.println("Test 3: " + getRedirectUrl("https://malicious.com/phishing"));
        System.out.println("Test 4: " + getRedirectUrl(""));
        System.out.println("Test 5: " + getRedirectUrl("https://example.com/path/to/page?query=value"));
    }
}
