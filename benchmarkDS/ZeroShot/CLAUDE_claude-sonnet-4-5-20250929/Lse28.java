
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse28 {
    // Compile the regular expression once when the class is loaded
    private static final Pattern URL_PATTERN = Pattern.compile("^https?://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}(/.*)?$");
    private static final String DEFAULT_URL = "https://default.example.com";
    
    public static String getRedirectUrl(String targetUrl) {
        if (targetUrl == null || targetUrl.isEmpty()) {
            return DEFAULT_URL;
        }
        
        Matcher matcher = URL_PATTERN.matcher(targetUrl);
        
        // If the target URL matches the pattern, use it as redirect target
        if (matcher.matches()) {
            return targetUrl;
        }
        
        // If the target URL does not match, use default URL
        return DEFAULT_URL;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "https://www.example.com/page",
            "http://test.com",
            "invalid-url",
            "ftp://wrong-protocol.com",
            "https://secure-site.org/path/to/resource"
        };
        
        System.out.println("Testing URL Redirect Logic:");
        for (String testUrl : testCases) {
            String result = getRedirectUrl(testUrl);
            System.out.println("Input: " + testUrl + " -> Redirect: " + result);
        }
    }
}
