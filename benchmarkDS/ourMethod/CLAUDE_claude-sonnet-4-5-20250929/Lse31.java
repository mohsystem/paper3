
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Arrays;
import java.util.List;

public class Lse31 {
    /**
     * Redirects to the URL if domain is example.com, otherwise returns 404.
     * 
     * @param url The URL to validate and redirect
     * @return Redirect message or 404 error
     */
    public static String redirectUrl(String url) {
        // Input validation: reject null or empty URLs
        if (url == null || url.trim().isEmpty()) {
            return "404 - Not Found";
        }
        
        // Limit URL length to prevent resource exhaustion
        if (url.length() > 2048) {
            return "404 - Not Found";
        }
        
        try {
            // Parse URL using URI for safe parsing - no code execution risk
            URI parsedUrl = new URI(url);
            
            // Get the network location (domain)
            String domain = parsedUrl.getHost();
            
            // Validate domain is present - fail closed
            if (domain == null || domain.trim().isEmpty()) {
                return "404 - Not Found";
            }
            
            // Normalize domain to lowercase for comparison - prevent bypass
            domain = domain.toLowerCase().trim();
            
            // Whitelist exact match only - prevent subdomain attacks
            if ("example.com".equals(domain)) {
                // Only redirect if scheme is http or https - prevent javascript: data: etc
                String scheme = parsedUrl.getScheme();
                if (scheme != null && (scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https"))) {
                    return "Redirect to: " + url;
                }
            }
            
            // Fail closed - reject anything not explicitly allowed
            return "404 - Not Found";
            
        } catch (URISyntaxException e) {
            // Generic error - do not leak internal details
            return "404 - Not Found";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        List<String> testUrls = Arrays.asList(
            "http://example.com/path",
            "https://example.com/secure",
            "http://notexample.com/path",
            "http://subdomain.example.com/path",
            "javascript:alert(1)"
        );
        
        for (String testUrl : testUrls) {
            System.out.println("URL: " + testUrl);
            System.out.println("Result: " + redirectUrl(testUrl));
            System.out.println();
        }
    }
}
