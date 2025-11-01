
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class Lse42 {
    private static Map<String, Map<String, String>> sessions = new HashMap<>();
    
    public static String maskSSN(String ssn) {
        if (ssn == null || ssn.length() < 4) return "***-**-****";
        return "***-**-" + ssn.substring(ssn.length() - 4);
    }
    
    public static String getInfo(String sessionId) {
        if (sessionId == null || sessionId.isEmpty()) {
            return "Unauthorized: No session found";
        }
        
        Map<String, String> session = sessions.get(sessionId);
        if (session == null) {
            return "Unauthorized: Invalid session";
        }
        
        String username = session.getOrDefault("username", "N/A");
        String email = session.getOrDefault("email", "N/A");
        String ssn = session.getOrDefault("ssn", "N/A");
        
        return String.format("Username: %s; Email: %s; SSN: %s", 
            escapeHtml(username), 
            escapeHtml(email), 
            maskSSN(ssn));
    }
    
    public static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        String session1 = UUID.randomUUID().toString();
        Map<String, String> userData1 = new HashMap<>();
        userData1.put("username", "john_doe");
        userData1.put("email", "john@example.com");
        userData1.put("ssn", "123-45-6789");
        sessions.put(session1, userData1);
        
        String session2 = UUID.randomUUID().toString();
        Map<String, String> userData2 = new HashMap<>();
        userData2.put("username", "jane_smith");
        userData2.put("email", "jane@example.com");
        userData2.put("ssn", "987-65-4321");
        sessions.put(session2, userData2);
        
        System.out.println("Test Case 1: " + getInfo(session1));
        System.out.println("Test Case 2: " + getInfo(session2));
        System.out.println("Test Case 3: " + getInfo(null));
        System.out.println("Test Case 4: " + getInfo("invalid-session"));
        System.out.println("Test Case 5: " + getInfo(""));
    }
}
