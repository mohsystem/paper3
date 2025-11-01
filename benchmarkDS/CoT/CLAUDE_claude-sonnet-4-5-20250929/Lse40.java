
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

public class Lse40 {
    private static final Map<String, Map<String, String>> sessions = new ConcurrentHashMap<>();
    
    public static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static String getLastFourDigits(String ssn) {
        if (ssn == null || ssn.length() < 4) return "****";
        return ssn.substring(ssn.length() - 4);
    }
    
    public static String generateInfoPage(String sessionId) {
        Map<String, String> sessionData = sessions.get(sessionId);
        
        if (sessionData == null) {
            return "<html><body><h1>Error: No session found</h1></body></html>";
        }
        
        String username = escapeHtml(sessionData.get("username"));
        String email = escapeHtml(sessionData.get("email"));
        String ssnLast4 = escapeHtml(getLastFourDigits(sessionData.get("ssn")));
        
        return "<html><head><title>User Info</title></head><body>" +
               "<h1>User Information</h1>" +
               "<p><strong>Username:</strong> " + username + "</p>" +
               "<p><strong>Email:</strong> " + email + "</p>" +
               "<p><strong>SSN (Last 4):</strong> " + ssnLast4 + "</p>" +
               "</body></html>";
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases - create sessions
        String session1 = UUID.randomUUID().toString();
        Map<String, String> data1 = new HashMap<>();
        data1.put("username", "john_doe");
        data1.put("email", "john@example.com");
        data1.put("ssn", "123456789");
        sessions.put(session1, data1);
        
        String session2 = UUID.randomUUID().toString();
        Map<String, String> data2 = new HashMap<>();
        data2.put("username", "jane<script>alert('xss')</script>");
        data2.put("email", "jane@test.com");
        data2.put("ssn", "987654321");
        sessions.put(session2, data2);
        
        String session3 = UUID.randomUUID().toString();
        Map<String, String> data3 = new HashMap<>();
        data3.put("username", "bob");
        data3.put("email", "bob@mail.com");
        data3.put("ssn", "111");
        sessions.put(session3, data3);
        
        String session4 = UUID.randomUUID().toString();
        Map<String, String> data4 = new HashMap<>();
        data4.put("username", "alice");
        data4.put("email", "alice@domain.com");
        data4.put("ssn", "555667788");
        sessions.put(session4, data4);
        
        String session5 = "invalid_session";
        
        // Test outputs
        System.out.println("Test 1: " + generateInfoPage(session1));
        System.out.println("\\nTest 2 (XSS): " + generateInfoPage(session2));
        System.out.println("\\nTest 3 (Short SSN): " + generateInfoPage(session3));
        System.out.println("\\nTest 4: " + generateInfoPage(session4));
        System.out.println("\\nTest 5 (Invalid): " + generateInfoPage(session5));
        
        // Start web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("\\nServer started on port 8080");
    }
    
    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = exchange.getRequestHeaders().getFirst("Session-ID");
            String response = generateInfoPage(sessionId);
            
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
            exchange.sendResponseHeaders(200, response.getBytes().length);
            
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }
}
