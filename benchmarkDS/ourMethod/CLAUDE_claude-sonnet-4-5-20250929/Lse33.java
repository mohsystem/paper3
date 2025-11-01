
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;

public class Lse33 {
    
    // Validates that the URL belongs to example.com domain
    // Returns true only if the URL is valid and from example.com
    private static boolean isValidExampleComUrl(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return false;
        }
        
        try {
            // Parse the URL to validate its structure
            URI uri = new URI(urlString);
            
            // Ensure the URL has a scheme (http or https only)
            String scheme = uri.getScheme();
            if (scheme == null || (!scheme.equalsIgnoreCase("http") && !scheme.equalsIgnoreCase("https"))) {
                return false;
            }
            
            // Get the host and validate it matches example.com
            String host = uri.getHost();
            if (host == null) {
                return false;
            }
            
            // Normalize host to lowercase for comparison
            host = host.toLowerCase();
            
            // Only allow exact match of example.com or subdomains of example.com
            // This prevents attacks like example.com.evil.com
            if (!host.equals("example.com") && !host.endsWith(".example.com")) {
                return false;
            }
            
            // Additional validation: ensure no @ symbol in authority (prevents user info injection)
            if (urlString.contains("@")) {
                return false;
            }
            
            return true;
            
        } catch (URISyntaxException e) {
            // Invalid URI syntax
            return false;
        }
    }
    
    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                // Only accept GET requests
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendResponse(exchange, 405, "Method Not Allowed");
                    return;
                }
                
                // Parse query parameters
                String query = exchange.getRequestURI().getQuery();
                String targetUrl = null;
                
                if (query != null) {
                    String[] params = query.split("&");
                    for (String param : params) {
                        String[] keyValue = param.split("=", 2);
                        if (keyValue.length == 2 && "url".equals(keyValue[0])) {
                            // Decode the URL parameter (basic decoding)
                            targetUrl = java.net.URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8);
                            break;
                        }
                    }
                }
                
                // Validate the URL
                if (targetUrl == null || targetUrl.isEmpty()) {
                    sendResponse(exchange, 400, "Error: URL parameter is required");
                    return;
                }
                
                // Check if URL is from example.com domain
                if (!isValidExampleComUrl(targetUrl)) {
                    sendResponse(exchange, 400, "Error: Invalid URL. Only example.com URLs are allowed.");
                    return;
                }
                
                // Perform the redirect (302 Found)
                exchange.getResponseHeaders().set("Location", targetUrl);
                exchange.sendResponseHeaders(302, -1);
                
            } catch (Exception e) {
                // Generic error message - don't leak internal details
                sendResponse(exchange, 500, "Error: Request processing failed");
            } finally {
                exchange.close();
            }
        }
        
        private void sendResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            byte[] response = message.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response);
            }
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Case 1 - Valid example.com URL:");
        System.out.println(isValidExampleComUrl("https://example.com/path") ? "PASS" : "FAIL");
        
        System.out.println("\\nTest Case 2 - Valid subdomain:");
        System.out.println(isValidExampleComUrl("https://sub.example.com/page") ? "PASS" : "FAIL");
        
        System.out.println("\\nTest Case 3 - Invalid domain:");
        System.out.println(!isValidExampleComUrl("https://evil.com") ? "PASS" : "FAIL");
        
        System.out.println("\\nTest Case 4 - Domain confusion attack:");
        System.out.println(!isValidExampleComUrl("https://example.com.evil.com") ? "PASS" : "FAIL");
        
        System.out.println("\\nTest Case 5 - Invalid scheme:");
        System.out.println(!isValidExampleComUrl("javascript:alert(1)") ? "PASS" : "FAIL");
        
        // Start web server on port 8000
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/redirect", new RedirectHandler());
        server.setExecutor(null);
        System.out.println("\\nServer started on port 8000");
        System.out.println("Try: http://localhost:8000/redirect?url=https://example.com/test");
        server.start();
    }
}
