
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;

public class Lse41 {
    // Simulated secure session store - in production use proper session management
    private static final Map<String, UserInfo> sessionStore = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    
    static class UserInfo {
        private final String username;
        private final String email;
        private final String ssnLast4;
        
        public UserInfo(String username, String email, String ssnLast4) {
            // Validate inputs to prevent injection attacks
            this.username = sanitizeOutput(username);
            this.email = sanitizeOutput(email);
            // Only store last 4 digits, validate format
            if (ssnLast4 != null && ssnLast4.matches("\\\\d{4}")) {
                this.ssnLast4 = ssnLast4;
            } else {
                this.ssnLast4 = "****";
            }
        }
        
        public String getUsername() { return username; }
        public String getEmail() { return email; }
        public String getSsnLast4() { return ssnLast4; }
    }
    
    // Sanitize output to prevent XSS attacks
    private static String sanitizeOutput(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                // Only allow GET requests
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendResponse(exchange, 405, "Method Not Allowed");
                    return;
                }
                
                // Retrieve session token from cookie - in production use secure session management
                String sessionId = getCookie(exchange, "sessionId");
                
                if (sessionId == null || !sessionStore.containsKey(sessionId)) {
                    // No valid session - return generic error
                    sendResponse(exchange, 401, "Unauthorized");
                    return;
                }
                
                UserInfo user = sessionStore.get(sessionId);
                
                // Build response with sanitized data
                StringBuilder response = new StringBuilder();
                response.append("<!DOCTYPE html>\\n<html>\\n<head>\\n");
                response.append("<meta charset=\\"UTF-8\\">\\n");
                response.append("<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'\\">\\n");
                response.append("<title>User Info</title>\\n</head>\\n<body>\\n");
                response.append("<h1>User Information</h1>\\n");
                response.append("<p>Username: ").append(user.getUsername()).append("</p>\\n");
                response.append("<p>Email: ").append(user.getEmail()).append("</p>\\n");
                response.append("<p>SSN (Last 4): ").append(user.getSsnLast4()).append("</p>\\n");
                response.append("</body>\\n</html>");
                
                // Set security headers
                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
                exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
                exchange.getResponseHeaders().set("X-XSS-Protection", "1; mode=block");
                
                byte[] responseBytes = response.toString().getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(200, responseBytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(responseBytes);
                }
            } catch (Exception e) {
                // Log error internally, return generic message
                sendResponse(exchange, 500, "Internal Server Error");
            }
        }
        
        private String getCookie(HttpExchange exchange, String name) {
            String cookies = exchange.getRequestHeaders().getFirst("Cookie");
            if (cookies != null) {
                for (String cookie : cookies.split(";")) {
                    String[] parts = cookie.trim().split("=", 2);
                    if (parts.length == 2 && name.equals(parts[0])) {
                        return parts[1];
                    }
                }
            }
            return null;
        }
        
        private void sendResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            byte[] bytes = message.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }
    
    public static void main(String[] args) {
        try {
            // Create test sessions
            String session1 = generateSessionId();
            String session2 = generateSessionId();
            String session3 = generateSessionId();
            String session4 = generateSessionId();
            String session5 = generateSessionId();
            
            sessionStore.put(session1, new UserInfo("john_doe", "john@example.com", "1234"));
            sessionStore.put(session2, new UserInfo("jane_smith", "jane@example.com", "5678"));
            sessionStore.put(session3, new UserInfo("bob_jones", "bob@example.com", "9012"));
            sessionStore.put(session4, new UserInfo("alice<script>", "alice@test.com", "3456"));
            sessionStore.put(session5, new UserInfo("charlie", "charlie@example.com", "7890"));
            
            // Create HTTP server on port 8000
            HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
            server.createContext("/info", new InfoHandler());
            server.setExecutor(null);
            server.start();
            
            System.out.println("Server started on port 8000");
            System.out.println("Test sessions created:");
            System.out.println("Session 1: " + session1);
            System.out.println("Session 2: " + session2);
            System.out.println("Session 3: " + session3);
            System.out.println("Session 4: " + session4);
            System.out.println("Session 5: " + session5);
            System.out.println("\\nAccess /info with Cookie: sessionId=<session_id>");
            
        } catch (IOException e) {
            System.err.println("Failed to start server: " + e.getMessage());
        }
    }
    
    private static String generateSessionId() {
        byte[] bytes = new byte[32];
        secureRandom.nextBytes(bytes);
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }
}
