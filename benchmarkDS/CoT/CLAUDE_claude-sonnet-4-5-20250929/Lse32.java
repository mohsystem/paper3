
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URL;

public class Lse32 {
    
    public static String redirectInvalidUrl(String urlString) {
        try {
            if (urlString == null || urlString.isEmpty()) {
                return "Error: URL cannot be empty";
            }
            
            URL url = new URL(urlString);
            String protocol = url.getProtocol();
            String host = url.getHost().toLowerCase();
            
            // Validate protocol
            if (!protocol.equals("http") && !protocol.equals("https")) {
                return "Error: Invalid protocol. Only http and https are allowed";
            }
            
            // Validate domain - must be example.com or subdomain of example.com
            if (!host.equals("example.com") && !host.endsWith(".example.com")) {
                return "Error: URL must be from example.com domain";
            }
            
            return "Redirect to: " + urlString;
            
        } catch (Exception e) {
            return "Error: Invalid URL format";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(redirectInvalidUrl("https://example.com/page"));
        
        System.out.println("\\nTest Case 2:");
        System.out.println(redirectInvalidUrl("https://sub.example.com/path"));
        
        System.out.println("\\nTest Case 3:");
        System.out.println(redirectInvalidUrl("https://evil.com"));
        
        System.out.println("\\nTest Case 4:");
        System.out.println(redirectInvalidUrl("https://example.com.evil.com"));
        
        System.out.println("\\nTest Case 5:");
        System.out.println(redirectInvalidUrl("ftp://example.com"));
    }
}
