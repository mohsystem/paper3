
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse29 {
    // Strict regex pattern that only matches URLs containing example.com
    // Anchored pattern to prevent bypass attempts with @ or other tricks
    private static final Pattern ALLOWED_PATTERN = Pattern.compile(
        "^https://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$"
    );
    
    // Maximum URL length to prevent DoS
    private static final int MAX_URL_LENGTH = 2048;

    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Cases:");
        System.out.println("1. Valid: " + isValidRedirect("https://example.com"));
        System.out.println("2. Valid: " + isValidRedirect("https://www.example.com/path"));
        System.out.println("3. Valid: " + isValidRedirect("https://subdomain.example.com/page"));
        System.out.println("4. Invalid: " + isValidRedirect("https://evil.com"));
        System.out.println("5. Invalid: " + isValidRedirect("https://evil.com@example.com"));
        
        // Start web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/redirect", new RedirectHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                // Only accept GET requests
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendError(exchange, 405, "Method Not Allowed");
                    return;
                }

                // Parse query parameters securely
                String query = exchange.getRequestURI().getQuery();
                String target = extractTargetParameter(query);

                // Validate and sanitize the target URL
                String redirectUrl = validateAndSanitizeUrl(target);

                // Send redirect response
                exchange.getResponseHeaders().add("Location", redirectUrl);
                // Add security headers to prevent additional attacks
                exchange.getResponseHeaders().add("X-Content-Type-Options", "nosniff");
                exchange.getResponseHeaders().add("X-Frame-Options", "DENY");
                exchange.sendResponseHeaders(302, -1);
            } catch (Exception e) {
                // Log error securely without exposing internal details
                System.err.println("Error processing request: " + e.getClass().getName());
                sendError(exchange, 500, "Internal Server Error");
            } finally {
                exchange.close();
            }
        }
    }

    // Extract target parameter from query string with validation
    private static String extractTargetParameter(String query) {
        if (query == null || query.isEmpty()) {
            return null;
        }

        // Limit query string length to prevent DoS
        if (query.length() > MAX_URL_LENGTH) {
            return null;
        }

        String[] params = query.split("&");
        for (String param : params) {
            String[] keyValue = param.split("=", 2);
            if (keyValue.length == 2 && "target".equals(keyValue[0])) {
                try {
                    // Decode URL parameter safely
                    return URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8.name());
                } catch (Exception e) {
                    return null;
                }
            }
        }
        return null;
    }

    // Validate and sanitize URL against whitelist pattern
    private static String validateAndSanitizeUrl(String target) {
        // Default safe redirect
        String safeDefault = "/";

        // Null or empty check
        if (target == null || target.trim().isEmpty()) {
            return safeDefault;
        }

        // Length validation to prevent DoS
        if (target.length() > MAX_URL_LENGTH) {
            return safeDefault;
        }

        // Normalize and validate against pattern
        String normalized = target.trim();
        
        // Check against whitelist pattern
        if (isValidRedirect(normalized)) {
            return normalized;
        }

        // If validation fails, redirect to safe default
        return safeDefault;
    }

    // Check if URL matches the allowed pattern
    private static boolean isValidRedirect(String url) {
        if (url == null) {
            return false;
        }
        
        Matcher matcher = ALLOWED_PATTERN.matcher(url);
        return matcher.matches();
    }

    // Send error response with minimal information disclosure
    private static void sendError(HttpExchange exchange, int statusCode, String message) 
            throws IOException {
        byte[] response = message.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=UTF-8");
        exchange.sendResponseHeaders(statusCode, response.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response);
        }
    }
}
