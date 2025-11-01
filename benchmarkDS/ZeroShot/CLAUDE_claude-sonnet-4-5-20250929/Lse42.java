
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.WebServlet;
import java.util.*;

public class Lse42 {
    
    // Simulated session store (in production, use proper session management)
    private static Map<String, Map<String, String>> sessionStore = new HashMap<>();
    
    // Simulate user authentication and session creation
    public static String createSession(String sessionId, String username, String email, String ssn) {
        Map<String, String> sessionData = new HashMap<>();
        sessionData.put("username", sanitize(username));
        sessionData.put("email", sanitize(email));
        sessionData.put("ssn", maskSSN(ssn));
        sessionData.put("authenticated", "true");
        sessionStore.put(sessionId, sessionData);
        return sessionId;
    }
    
    // Route handler for /info
    public static String info(String sessionId) {
        if (sessionId == null || !sessionStore.containsKey(sessionId)) {
            return "Error: Invalid or expired session. Please log in.";
        }
        
        Map<String, String> sessionData = sessionStore.get(sessionId);
        
        if (!"true".equals(sessionData.get("authenticated"))) {
            return "Error: Unauthorized access.";
        }
        
        String username = sessionData.getOrDefault("username", "N/A");
        String email = sessionData.getOrDefault("email", "N/A");
        String ssn = sessionData.getOrDefault("ssn", "N/A");
        
        return String.format("User Information:\\nUsername: %s\\nEmail: %s\\nSSN: %s", 
                           username, email, ssn);
    }
    
    // Sanitize input to prevent XSS
    private static String sanitize(String input) {
        if (input == null) return "";
        return input.replaceAll("[<>\\"'&]", "")
                   .replaceAll("javascript:", "")
                   .replaceAll("onerror=", "")
                   .replaceAll("onclick=", "");
    }
    
    // Mask SSN for security (show only last 4 digits)
    private static String maskSSN(String ssn) {
        if (ssn == null || ssn.length() < 4) return "***";
        return "XXX-XX-" + ssn.substring(ssn.length() - 4);
    }
    
    public static void main(String[] args) {
        System.out.println("Testing /info route with session management\\n");
        
        // Test case 1: Valid session
        String session1 = createSession("sess001", "john_doe", "john@example.com", "123456789");
        System.out.println("Test 1 - Valid session:");
        System.out.println(info("sess001"));
        System.out.println();
        
        // Test case 2: Another valid session
        String session2 = createSession("sess002", "jane_smith", "jane@example.com", "987654321");
        System.out.println("Test 2 - Another valid session:");
        System.out.println(info("sess002"));
        System.out.println();
        
        // Test case 3: Invalid session ID
        System.out.println("Test 3 - Invalid session:");
        System.out.println(info("invalid_session"));
        System.out.println();
        
        // Test case 4: Null session ID
        System.out.println("Test 4 - Null session:");
        System.out.println(info(null));
        System.out.println();
        
        // Test case 5: XSS attempt in username
        String session3 = createSession("sess003", "<script>alert('xss')</script>user", 
                                       "test@example.com", "555667777");
        System.out.println("Test 5 - XSS attempt (sanitized):");
        System.out.println(info("sess003"));
    }
}
