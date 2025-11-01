
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {
    // Compile the regular expression once when the class is loaded
    private static final Pattern URL_PATTERN = Pattern.compile("^https?://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}(/.*)?$");
    private static final String DEFAULT_URL = "https://www.default.com";
    
    public static String getRedirectUrl(String targetUrl) {
        Matcher matcher = URL_PATTERN.matcher(targetUrl);
        
        // If the target URL matches the pattern, use it as redirect target
        if (matcher.matches()) {
            return targetUrl;
        }
        
        // If the target URL does not match, use default URL
        return DEFAULT_URL;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid HTTP URL
        System.out.println("Test 1: " + getRedirectUrl("http://www.example.com"));
        
        // Test case 2: Valid HTTPS URL with path
        System.out.println("Test 2: " + getRedirectUrl("https://www.example.com/path/to/page"));
        
        // Test case 3: Valid URL with subdomain
        System.out.println("Test 3: " + getRedirectUrl("https://subdomain.example.com"));
        
        // Test case 4: Invalid URL (missing protocol)
        System.out.println("Test 4: " + getRedirectUrl("www.example.com"));
        
        // Test case 5: Invalid URL (random text)
        System.out.println("Test 5: " + getRedirectUrl("not-a-valid-url"));
    }
}
