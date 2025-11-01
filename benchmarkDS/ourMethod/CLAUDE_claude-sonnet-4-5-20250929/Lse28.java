
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {
    // Compile regex once at class load time
    // Pattern is immutable and thread-safe
    private static final Pattern URL_PATTERN = Pattern.compile("^https://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}/[a-zA-Z0-9/_-]*$");
    
    // Default URL is a constant, not constructed from user input
    private static final String DEFAULT_URL = "https://example.com/default";
    
    /**
     * Validates and returns a redirect URL.
     * The regex validates format only; matched URL is used directly without reconstruction.
     * 
     * @param targetUrl The URL to validate
     * @return The validated target URL or default URL
     */
    public static String getRedirectUrl(String targetUrl) {
        // Input validation: check for null and excessive length
        if (targetUrl == null || targetUrl.length() > 2048) {
            return DEFAULT_URL;
        }
        
        // Validate against whitelist pattern
        // The regex is used only for validation, not construction
        Matcher matcher = URL_PATTERN.matcher(targetUrl);
        
        // If pattern matches, return the original validated input
        // Not constructing URL from regex groups - using validated input directly
        if (matcher.matches()) {
            return targetUrl;
        }
        
        // If validation fails, return safe default
        return DEFAULT_URL;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid HTTPS URL
        String test1 = "https://trusted-site.com/page";
        System.out.println("Test 1: " + getRedirectUrl(test1));
        
        // Test case 2: Valid HTTPS URL with path
        String test2 = "https://secure.example.org/user/profile";
        System.out.println("Test 2: " + getRedirectUrl(test2));
        
        // Test case 3: Invalid - HTTP instead of HTTPS
        String test3 = "http://untrusted.com/page";
        System.out.println("Test 3: " + getRedirectUrl(test3));
        
        // Test case 4: Invalid - contains special characters
        String test4 = "https://evil.com/redirect?url=javascript:alert(1)";
        System.out.println("Test 4: " + getRedirectUrl(test4));
        
        // Test case 5: Null input
        String test5 = null;
        System.out.println("Test 5: " + getRedirectUrl(test5));
    }
}
