
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse134 {
    // Whitelist pattern for username - only alphanumeric and hyphen/underscore
    private static final Pattern VALID_USERNAME = Pattern.compile("^[a-zA-Z0-9_-]{1,50}$");
    
    // HTML escape function to prevent XSS
    private static String escape(String input) {
        if (input == null) {
            return "";
        }
        // Validate input length to prevent DoS
        if (input.length() > 1000) {
            return "";
        }
        
        StringBuilder escaped = new StringBuilder();
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '&':
                    escaped.append("&amp;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\\'':
                    escaped.append("&#x27;");
                    break;
                case '/':
                    escaped.append("&#x2F;");
                    break;
                default:
                    escaped.append(c);
            }
        }
        return escaped.toString();
    }
    
    // Hello function that returns greeting with escaped username
    private static String hello(String username) {
        // Input validation - reject null or empty
        if (username == null || username.isEmpty()) {
            return "Hello Guest";
        }
        
        // Validate username against whitelist pattern
        if (!VALID_USERNAME.matcher(username).matches()) {
            return "Hello Guest";
        }
        
        // Escape the username to prevent XSS
        String escapedUsername = escape(username);
        return "Hello " + escapedUsername;
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test 1: " + hello("john"));
        System.out.println("Test 2: " + hello("alice_123"));
        System.out.println("Test 3: " + hello("<script>alert('xss')</script>"));
        System.out.println("Test 4: " + hello("user-name"));
        System.out.println("Test 5: " + hello(null));
        
        // Create HTTP server on port 8000
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        
        // Map /hello/ path to handler
        server.createContext("/hello/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    String path = exchange.getRequestURI().getPath();
                    
                    // Extract username from path - only accept exact pattern /hello/username
                    if (path.startsWith("/hello/")) {
                        String username = path.substring(7); // Remove "/hello/" prefix
                        
                        // Remove any additional path components for security
                        int slashIndex = username.indexOf('/');
                        if (slashIndex > 0) {
                            username = username.substring(0, slashIndex);
                        }
                        
                        // Get greeting with validated and escaped username
                        String response = hello(username);
                        
                        // Set secure headers
                        exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                        exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
                        exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
                        
                        byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
                        exchange.sendResponseHeaders(200, responseBytes.length);
                        
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(responseBytes);
                        }
                    } else {
                        // Invalid path
                        String response = "Not Found";
                        byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
                        exchange.sendResponseHeaders(404, responseBytes.length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(responseBytes);
                        }
                    }
                } catch (Exception e) {
                    // Generic error response - do not leak internal details
                    String response = "Internal Server Error";
                    byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(500, responseBytes.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(responseBytes);
                    }
                }
            }
        });
        
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
    }
}
