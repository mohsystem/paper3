
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Lse32 {
    private static final Set<String> ALLOWED_DOMAINS = new HashSet<>(Arrays.asList("example.com", "www.example.com"));
    
    public static String redirectInvalidUrl(String url) {
        try {
            if (url == null || url.trim().isEmpty()) {
                return "Error: URL cannot be empty";
            }
            
            // Decode URL to prevent encoding bypasses
            String decodedUrl = URLDecoder.decode(url, StandardCharsets.UTF_8.toString());
            
            // Parse the URL
            URI uri = new URI(decodedUrl);
            
            // Extract host
            String host = uri.getHost();
            
            if (host == null) {
                return "Error: Invalid URL format";
            }
            
            // Normalize host to lowercase
            host = host.toLowerCase();
            
            // Check if host is in allowed list
            if (ALLOWED_DOMAINS.contains(host)) {
                return "Redirect to: " + decodedUrl;
            } else {
                return "Error: URL is not from an allowed domain";
            }
            
        } catch (Exception e) {
            return "Error: Invalid URL - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + redirectInvalidUrl("https://example.com/page"));
        System.out.println("Test 2: " + redirectInvalidUrl("https://malicious.com/page"));
        System.out.println("Test 3: " + redirectInvalidUrl("https://www.example.com/secure"));
        System.out.println("Test 4: " + redirectInvalidUrl(""));
        System.out.println("Test 5: " + redirectInvalidUrl("https://example.com.malicious.com/fake"));
    }
}
