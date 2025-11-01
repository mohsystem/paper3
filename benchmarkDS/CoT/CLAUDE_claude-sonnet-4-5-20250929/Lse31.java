
import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {
    
    public static String redirect_url(String url) {
        if (url == null || url.trim().isEmpty()) {
            return "404 Error: Invalid URL";
        }
        
        try {
            URI parsedUrl = new URI(url);
            String domain = parsedUrl.getHost();
            
            // Exact domain match (case-insensitive) - no subdomains allowed
            if (domain != null && domain.equalsIgnoreCase("example.com")) {
                return "Redirect to: " + url;
            } else {
                return "404 Error: Domain not allowed";
            }
        } catch (URISyntaxException e) {
            return "404 Error: Invalid URL format";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + redirect_url("http://example.com/path"));
        System.out.println("Test 2: " + redirect_url("https://example.com/secure"));
        System.out.println("Test 3: " + redirect_url("http://evil.com/malicious"));
        System.out.println("Test 4: " + redirect_url("http://subdomain.example.com/path"));
        System.out.println("Test 5: " + redirect_url("invalid-url"));
    }
}
