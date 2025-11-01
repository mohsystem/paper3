
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse42 {
    // Session storage (in production, use proper session management)
    private static final Map<String, Map<String, String>> sessionStore = new HashMap<>();
    
    public static void main(String[] args) throws IOException {
        // Test cases
        testInfoEndpoint();
        
        // Start server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        System.out.println("Server started on port 8000");
        server.start();
    }
    
    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Only allow GET requests - fail closed for other methods
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }
            
            // Extract session ID from cookie (simplified for example)
            String sessionId = extractSessionId(exchange);
            
            if (sessionId == null || sessionId.isEmpty()) {
                // No session - return generic error, don't leak details\n                sendResponse(exchange, 401, "Unauthorized");\n                return;\n            }\n            \n            String response = info(sessionId);\n            sendResponse(exchange, 200, response);\n        }\n        \n        private String extractSessionId(HttpExchange exchange) {\n            String cookie = exchange.getRequestHeaders().getFirst("Cookie");\n            if (cookie != null && cookie.startsWith("sessionId=")) {\n                return cookie.substring("sessionId=".length());\n            }\n            return null;\n        }\n        \n        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {\n            byte[] bytes = response.getBytes(StandardCharsets.UTF_8);\n            exchange.sendResponseHeaders(statusCode, bytes.length);\n            try (OutputStream os = exchange.getResponseBody()) {\n                os.write(bytes);\n            }\n        }\n    }\n    \n    /**\n     * Returns user information from session.\n     * SECURITY: Never log sensitive data like SSN or email.\n     * Returns only redacted/masked information to prevent exposure.\n     */\n    public static String info(String sessionId) {\n        // Validate session ID input - reject if null or malformed\n        if (sessionId == null || sessionId.trim().isEmpty()) {\n            // Return generic error, don't leak session details
            return "Error: Invalid session";
        }
        
        Map<String, String> sessionData = sessionStore.get(sessionId);
        
        if (sessionData == null) {
            // Session not found - return generic error
            return "Error: Session not found";
        }
        
        // Retrieve session variables - treat all as untrusted
        String username = sessionData.getOrDefault("username", "");
        String email = sessionData.getOrDefault("email", "");
        String ssn = sessionData.getOrDefault("ssn", "");
        
        // SECURITY: Redact sensitive information (SSN, email)
        // Never return full SSN or email in responses
        String maskedEmail = maskEmail(email);
        String maskedSsn = maskSsn(ssn);
        
        // Return sanitized data using constant format string
        return String.format("Username: %s, Email: %s, SSN: %s", 
            sanitizeForOutput(username), maskedEmail, maskedSsn);
    }
    
    /**
     * Masks email to show only first character and domain
     * SECURITY: Prevents full email exposure
     */
    private static String maskEmail(String email) {
        if (email == null || email.isEmpty() || !email.contains("@")) {
            return "***";
        }
        String[] parts = email.split("@");
        if (parts.length != 2 || parts[0].isEmpty()) {
            return "***";
        }
        return parts[0].charAt(0) + "***@" + parts[1];
    }
    
    /**
     * Masks SSN to show only last 4 digits
     * SECURITY: Prevents full SSN exposure
     */
    private static String maskSsn(String ssn) {
        if (ssn == null || ssn.length() < 4) {
            return "***";
        }
        return "XXX-XX-" + ssn.substring(ssn.length() - 4);
    }
    
    /**
     * Sanitizes output to prevent injection attacks
     * SECURITY: Escapes special characters
     */
    private static String sanitizeForOutput(String input) {
        if (input == null) {
            return "";
        }
        // Remove or escape potentially dangerous characters
        return input.replaceAll("[<>&\\"\\']", "");
    }
    
    // Helper method to setup test sessions
    private static void setupTestSession(String sessionId, String username, String email, String ssn) {
        Map<String, String> data = new HashMap<>();
        data.put("username", username);
        data.put("email", email);
        data.put("ssn", ssn);
        sessionStore.put(sessionId, data);
    }
    
    private static void testInfoEndpoint() {
        System.out.println("=== Test Cases ===");
        
        // Test case 1: Valid session
        setupTestSession("session1", "john_doe", "john@example.com", "123456789");
        System.out.println("Test 1: " + info("session1"));
        
        // Test case 2: Another valid session
        setupTestSession("session2", "jane_smith", "jane@test.org", "987654321");
        System.out.println("Test 2: " + info("session2"));
        
        // Test case 3: Invalid/non-existent session
        System.out.println("Test 3: " + info("invalid_session"));
        
        // Test case 4: Null session ID
        System.out.println("Test 4: " + info(null));
        
        // Test case 5: Empty session ID
        System.out.println("Test 5: " + info(""));
        
        System.out.println("==================\\n");
    }
}
