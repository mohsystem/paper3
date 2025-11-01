
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse132 {
    // Whitelist pattern for username - only alphanumeric and underscore allowed
    // This prevents path traversal, XSS, and other injection attacks
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{1,50}$");
    private static final int MAX_USERNAME_LENGTH = 50;
    
    // HTML escaping to prevent XSS attacks
    private static String escapeHtml(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    // Validate username against whitelist pattern
    private static boolean isValidUsername(String username) {
        if (username == null || username.isEmpty() || username.length() > MAX_USERNAME_LENGTH) {
            return false;
        }
        return USERNAME_PATTERN.matcher(username).matches();
    }
    
    // Render template with secure variable substitution
    private static String renderTemplate(String username) throws IOException {
        // Validate username before processing
        if (!isValidUsername(username)) {
            throw new IllegalArgumentException("Invalid username format");
        }
        
        // Escape username to prevent XSS
        String safeUsername = escapeHtml(username);
        
        // Read template file from a controlled location
        // Using getResource to prevent path traversal
        InputStream templateStream = Lse132.class.getResourceAsStream("/hello.html");
        
        String template;
        if (templateStream != null) {
            template = new String(templateStream.readAllBytes(), StandardCharsets.UTF_8);
            templateStream.close();
        } else {
            // Fallback template if file not found
            template = "<!DOCTYPE html><html><body><h1>Hello, {{ username }}!</h1></body></html>";
        }
        
        // Replace placeholder with escaped username
        return template.replace("{{ username }}", safeUsername);
    }
    
    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            
            // Extract username from URL path /hello/<username>
            Pattern urlPattern = Pattern.compile("^/hello/([^/]+)$");
            Matcher matcher = urlPattern.matcher(path);
            
            if (matcher.matches()) {
                String username = matcher.group(1);
                
                try {
                    // Validate and render template
                    String response = renderTemplate(username);
                    
                    // Set secure headers
                    exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                    exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
                    exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
                    exchange.getResponseHeaders().set("Content-Security-Policy", "default-src 'self'");
                    
                    byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(200, responseBytes.length);
                    
                    OutputStream os = exchange.getResponseBody();
                    os.write(responseBytes);
                    os.close();
                    
                } catch (IllegalArgumentException e) {
                    // Generic error message - don't leak internal details\n                    String errorResponse = "Invalid request";\n                    byte[] errorBytes = errorResponse.getBytes(StandardCharsets.UTF_8);\n                    exchange.sendResponseHeaders(400, errorBytes.length);\n                    OutputStream os = exchange.getResponseBody();\n                    os.write(errorBytes);\n                    os.close();\n                }\n            } else {\n                // 404 for invalid paths\n                String notFound = "Not Found";\n                byte[] notFoundBytes = notFound.getBytes(StandardCharsets.UTF_8);\n                exchange.sendResponseHeaders(404, notFoundBytes.length);\n                OutputStream os = exchange.getResponseBody();\n                os.write(notFoundBytes);\n                os.close();\n            }\n        }\n    }\n    \n    public static void main(String[] args) throws IOException {\n        // Test cases\n        System.out.println("Test Case 1 - Valid alphanumeric username:");\n        System.out.println(renderTemplate("john123"));\n        System.out.println();\n        \n        System.out.println("Test Case 2 - Valid username with underscore:");\n        System.out.println(renderTemplate("user_name"));\n        System.out.println();\n        \n        System.out.println("Test Case 3 - Simple username:");\n        System.out.println(renderTemplate("alice"));\n        System.out.println();\n        \n        System.out.println("Test Case 4 - XSS attempt (will be escaped):");\n        try {\n            System.out.println(renderTemplate("<script>alert('xss')</script>"));\n        } catch (IllegalArgumentException e) {\n            System.out.println("Rejected: " + e.getMessage());\n        }\n        System.out.println();\n        \n        System.out.println("Test Case 5 - Path traversal attempt:");\n        try {\n            System.out.println(renderTemplate("../etc/passwd"));\n        } catch (IllegalArgumentException e) {\n            System.out.println("Rejected: " + e.getMessage());\n        }\n        \n        // Start HTTP server on port 8000\n        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);\n        server.createContext("/hello/", new HelloHandler());\n        server.setExecutor(null);\n        server.start();\n        System.out.println("\
Server started on http://localhost:8000");
    }
}
