
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.charset.StandardCharsets;

public class Lse113 {
    
    // Security: Using InetAddress.isReachable() instead of system ping command
    // This prevents command injection vulnerabilities (CWE-78)
    // Input validation ensures only valid hostnames/IPs are processed
    public static String ping(String url) {
        // Input validation: check for null or empty
        if (url == null || url.trim().isEmpty()) {
            return "Error: Invalid URL provided";
        }
        
        // Sanitize input: remove whitespace and validate format
        url = url.trim();
        
        // Validate URL format - reject suspicious characters that could be used for injection
        // Only allow alphanumeric, dots, hyphens, and colons (for IPv6)
        if (!url.matches("^[a-zA-Z0-9.:\\\\-]+$")) {
            return "Error: Invalid URL format";
        }
        
        // Limit URL length to prevent resource exhaustion
        if (url.length() > 253) {
            return "Error: URL too long";
        }
        
        try {
            // Security: Using Java's built-in InetAddress.isReachable() method
            // This is safer than executing system commands
            InetAddress address = InetAddress.getByName(url);
            
            // Timeout set to 5000ms to prevent indefinite hanging
            boolean isReachable = address.isReachable(5000);
            
            if (isReachable) {
                return url + " is alive";
            } else {
                return url + " is not alive";
            }
        } catch (UnknownHostException e) {
            // Generic error message to avoid leaking internal details
            return "Error: Unable to resolve host";
        } catch (IOException e) {
            // Generic error message
            return "Error: Network error occurred";
        }
    }
    
    // HTTP handler that binds ping functionality to /ping endpoint
    static class PingHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Only accept GET requests
            if (!"GET".equals(exchange.getRequestMethod())) {
                String response = "Method not allowed";
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
                return;
            }
            
            // Extract URL parameter from query string
            String query = exchange.getRequestURI().getQuery();
            String url = null;
            
            if (query != null && query.startsWith("url=")) {
                url = query.substring(4);
            }
            
            String result;
            if (url == null || url.isEmpty()) {
                result = "Error: URL parameter required";
            } else {
                result = ping(url);
            }
            
            // Send response
            exchange.sendResponseHeaders(200, result.length());
            OutputStream os = exchange.getResponseBody();
            os.write(result.getBytes(StandardCharsets.UTF_8));
            os.close();
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Case 1: Valid domain");
        System.out.println(ping("google.com"));
        
        System.out.println("\\nTest Case 2: Valid IP");
        System.out.println(ping("8.8.8.8"));
        
        System.out.println("\\nTest Case 3: Invalid domain");
        System.out.println(ping("thisdoesnotexist999.invalid"));
        
        System.out.println("\\nTest Case 4: Empty string");
        System.out.println(ping(""));
        
        System.out.println("\\nTest Case 5: Malicious input attempt");
        System.out.println(ping("google.com && cat /etc/passwd"));
        
        // Start HTTP server on port 8000
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/ping", new PingHandler());
        server.setExecutor(null);
        System.out.println("\\nServer started on http://localhost:8000/ping?url=<target>");
        server.start();
    }
}
