
import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Map;

public class Lse33 {
    
    public static Map<String, Object> redirectInvalidUrl(String urlString) {
        Map<String, Object> response = new HashMap<>();
        
        if (urlString == null || urlString.isEmpty()) {
            response.put("status", "error");
            response.put("message", "URL parameter is required");
            return response;
        }
        
        try {
            URI uri = new URI(urlString);
            String host = uri.getHost();
            
            if (host == null) {
                response.put("status", "error");
                response.put("message", "Invalid URL format");
                return response;
            }
            
            // Normalize host to lowercase for comparison
            host = host.toLowerCase();
            
            // Check if domain is exactly example.com or subdomain of example.com
            if (host.equals("example.com") || host.endsWith(".example.com")) {
                response.put("status", "redirect");
                response.put("url", urlString);
                response.put("message", "Redirecting to: " + urlString);
            } else {
                response.put("status", "error");
                response.put("message", "Invalid domain. Only example.com domain is allowed");
            }
            
        } catch (URISyntaxException e) {
            response.put("status", "error");
            response.put("message", "Invalid URL format: " + e.getMessage());
        }
        
        return response;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid example.com URL
        System.out.println("Test 1: " + redirectInvalidUrl("https://example.com/page"));
        
        // Test case 2: Valid subdomain of example.com
        System.out.println("Test 2: " + redirectInvalidUrl("https://sub.example.com/path"));
        
        // Test case 3: Invalid domain
        System.out.println("Test 3: " + redirectInvalidUrl("https://malicious.com/page"));
        
        // Test case 4: Invalid URL format
        System.out.println("Test 4: " + redirectInvalidUrl("not-a-valid-url"));
        
        // Test case 5: Empty/null URL
        System.out.println("Test 5: " + redirectInvalidUrl(null));
    }
}
