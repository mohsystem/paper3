
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse41 {
    private static class User {
        String username;
        String email;
        String ssnLast4;
        String sessionToken;
        
        User(String username, String email, String ssnLast4, String token) {
            this.username = escapeHtml(username);
            this.email = escapeHtml(email);
            // Only store last 4 digits, validate format
            if (ssnLast4 != null && ssnLast4.matches("\\\\d{4}")) {
                this.ssnLast4 = ssnLast4;
            } else {
                this.ssnLast4 = "****";
            }
            this.sessionToken = token;
        }
    }
    
    private static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    private static User authenticateUser(String token) {
        // Simulated user database - in production, use secure database
        if ("valid_token_123".equals(token)) {
            return new User("john_doe", "john@example.com", "5678", token);
        }
        return null;
    }
    
    static class InfoHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;
            
            // Simple token-based auth (use proper OAuth/JWT in production)
            String authHeader = exchange.getRequestHeaders().getFirst("Authorization");
            User user = authenticateUser(authHeader);
            
            if (user == null) {
                statusCode = 401;
                response = "<html><body><h1>Unauthorized</h1></body></html>";
            } else {
                statusCode = 200;
                response = String.format(
                    "<html><head><meta charset='UTF-8'></head><body>" +
                    "<h1>User Information</h1>" +
                    "<p>Username: %s</p>" +
                    "<p>Email: %s</p>" +
                    "<p>SSN Last 4: ****-%s</p>" +
                    "</body></html>",
                    user.username, user.email, user.ssnLast4
                );
            }
            
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes(StandardCharsets.UTF_8));
            os.close();
        }
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/info", new InfoHandler());
        
        System.out.println("Testing with 5 scenarios:");
        System.out.println("1. Valid token: curl -H 'Authorization: valid_token_123' http://localhost:8080/info");
        System.out.println("2. Invalid token: curl -H 'Authorization: invalid' http://localhost:8080/info");
        System.out.println("3. No token: curl http://localhost:8080/info");
        System.out.println("4. XSS attempt in username (blocked by escapeHtml)");
        System.out.println("5. Valid user data display with masked SSN");
        
        server.start();
        System.out.println("Server started on port 8080. Press Ctrl+C to stop.");
    }
}
