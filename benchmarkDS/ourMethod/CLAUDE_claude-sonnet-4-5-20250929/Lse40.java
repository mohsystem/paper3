
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class Lse40 {
    // Secure session store with timeout capability
    private static final Map<String, SessionData> sessions = new HashMap<>();
    private static final long SESSION_TIMEOUT = 30 * 60 * 1000; // 30 minutes
    
    static class SessionData {
        String username;
        String email;
        String ssn; // Store full SSN encrypted in production
        long lastAccess;
        
        SessionData(String username, String email, String ssn) {
            this.username = username;
            this.email = email;
            this.ssn = ssn;
            this.lastAccess = System.currentTimeMillis();
        }
        
        boolean isExpired() {
            return System.currentTimeMillis() - lastAccess > SESSION_TIMEOUT;
        }
        
        void updateAccess() {
            this.lastAccess = System.currentTimeMillis();
        }
    }
    
    // Sanitize output to prevent XSS attacks
    private static String sanitizeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    // Extract last 4 digits of SSN securely
    private static String getLastFourDigits(String ssn) {
        if (ssn == null || ssn.length() < 4) {
            return "****"; // Return masked value if invalid
        }
        // Only return last 4 digits, mask rest
        return ssn.substring(ssn.length() - 4);
    }
    
    // Validate SSN format (basic validation)
    private static boolean isValidSSN(String ssn) {
        if (ssn == null) return false;
        // Remove hyphens and validate format XXX-XX-XXXX or XXXXXXXXX
        String cleaned = ssn.replaceAll("-", "");
        return cleaned.matches("\\\\d{9}");
    }
    
    // Get session ID from cookie (simplified - in production use secure cookies)
    private static String getSessionId(HttpExchange exchange) {
        String cookie = exchange.getRequestHeaders().getFirst("Cookie");
        if (cookie != null && cookie.startsWith("sessionId=")) {
            return cookie.substring(10);
        }
        return null;
    }
    
    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Only allow GET requests
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                String response = "Method Not Allowed";
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
                return;
            }
            
            // Retrieve session ID from cookie
            String sessionId = getSessionId(exchange);
            
            if (sessionId == null || !sessions.containsKey(sessionId)) {
                // No valid session - return error
                String response = "Unauthorized: No valid session found";
                exchange.sendResponseHeaders(401, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
                return;
            }
            
            SessionData session = sessions.get(sessionId);
            
            // Check if session is expired
            if (session.isExpired()) {
                sessions.remove(sessionId);
                String response = "Unauthorized: Session expired";
                exchange.sendResponseHeaders(401, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
                return;
            }
            
            // Update session access time
            session.updateAccess();
            
            // Sanitize all output to prevent XSS
            String safeUsername = sanitizeHtml(session.username);
            String safeEmail = sanitizeHtml(session.email);
            String lastFour = getLastFourDigits(session.ssn);
            
            // Build response with proper content type
            String response = String.format(
                "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">" +
                "<title>User Info</title></head><body>" +
                "<h1>User Information</h1>" +
                "<p>Username: %s</p>" +
                "<p>Email: %s</p>" +
                "<p>SSN (last 4): ***-**-%s</p>" +
                "</body></html>",
                safeUsername, safeEmail, lastFour
            );
            
            // Set security headers
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
            exchange.getResponseHeaders().set("Content-Security-Policy", "default-src 'self'");
            
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes(StandardCharsets.UTF_8));
            os.close();
        }
    }
    
    // Helper method to create a test session
    private static String createSession(String username, String email, String ssn) {
        if (!isValidSSN(ssn)) {
            throw new IllegalArgumentException("Invalid SSN format");
        }
        String sessionId = UUID.randomUUID().toString();
        sessions.put(sessionId, new SessionData(username, email, ssn));
        return sessionId;
    }
    
    public static void main(String[] args) throws Exception {
        // Create test sessions for demonstration
        String session1 = createSession("john_doe", "john@example.com", "123-45-6789");
        String session2 = createSession("jane_smith", "jane@example.com", "987-65-4321");
        String session3 = createSession("bob_jones", "bob@example.com", "555-12-3456");
        String session4 = createSession("alice_williams", "alice@example.com", "111-22-3333");
        String session5 = createSession("charlie_brown", "charlie@example.com", "999-88-7777");
        
        System.out.println("Test session IDs created:");
        System.out.println("Session 1: " + session1);
        System.out.println("Session 2: " + session2);
        System.out.println("Session 3: " + session3);
        System.out.println("Session 4: " + session4);
        System.out.println("Session 5: " + session5);
        
        // Create HTTP server on port 8000
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        server.start();
        
        System.out.println("\\nServer started on http://localhost:8000");
        System.out.println("Access /info endpoint with Cookie header: sessionId=<session_id>");
        System.out.println("Press Ctrl+C to stop");
    }
}
