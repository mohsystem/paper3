
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;

public class Lse32 {
    
    // Validates if the URL is from example.com domain
    private static boolean isValidExampleUrl(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return false;
        }
        
        try {
            // Parse and validate URL structure
            URI uri = new URI(urlString);
            
            // Ensure absolute URL with scheme
            if (!uri.isAbsolute() || uri.getScheme() == null) {
                return false;
            }
            
            // Only allow https for security
            if (!"https".equalsIgnoreCase(uri.getScheme())) {
                return false;
            }
            
            String host = uri.getHost();
            if (host == null) {
                return false;
            }
            
            // Normalize host to lowercase for comparison
            host = host.toLowerCase();
            
            // Check if host is exactly example.com or subdomain of example.com
            // Prevent bypass attempts like "example.com.evil.com"
            if (host.equals("example.com") || host.endsWith(".example.com")) {
                return true;
            }
            
            return false;
        } catch (URISyntaxException e) {
            // Invalid URI syntax
            return false;
        }
    }
    
    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            
            // Extract URL parameter from path /redirect/<url>
            String[] parts = path.split("/", 4);
            
            if (parts.length < 3 || parts[2] == null || parts[2].isEmpty()) {
                sendError(exchange, "Invalid request format");
                return;
            }
            
            // Get URL from path (everything after /redirect/)
            String urlParam = path.substring("/redirect/".length());
            
            // Validate the URL
            if (isValidExampleUrl(urlParam)) {
                // Valid example.com URL - redirect
                exchange.getResponseHeaders().set("Location", urlParam);
                exchange.sendResponseHeaders(302, -1);
            } else {
                // Invalid URL - return error
                sendError(exchange, "Error: Invalid URL. Only example.com URLs are allowed.");
            }
            
            exchange.close();
        }
        
        private void sendError(HttpExchange exchange, String message) throws IOException {
            byte[] response = message.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            exchange.sendResponseHeaders(400, response.length);
            OutputStream os = exchange.getResponseBody();
            os.write(response);
            os.close();
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Case 1 - Valid example.com URL:");
        System.out.println(isValidExampleUrl("https://example.com/page"));
        
        System.out.println("\\nTest Case 2 - Valid subdomain:");
        System.out.println(isValidExampleUrl("https://sub.example.com/path"));
        
        System.out.println("\\nTest Case 3 - Invalid domain:");
        System.out.println(isValidExampleUrl("https://evil.com"));
        
        System.out.println("\\nTest Case 4 - Bypass attempt:");
        System.out.println(isValidExampleUrl("https://example.com.evil.com"));
        
        System.out.println("\\nTest Case 5 - HTTP not allowed:");
        System.out.println(isValidExampleUrl("http://example.com"));
        
        // Start server on port 8000
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/redirect/", new RedirectHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("\\nServer started on port 8000");
    }
}
