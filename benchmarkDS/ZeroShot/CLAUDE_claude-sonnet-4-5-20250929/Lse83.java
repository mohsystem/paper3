
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse83 {
    
    public static String processPayload(Map<String, String> payload) {
        if (payload == null) {
            return "Error: Invalid payload";
        }
        
        if (payload.containsKey("name")) {
            String name = payload.get("name");
            // Sanitize input to prevent XSS attacks
            if (name != null) {
                name = sanitizeInput(name);
                return "Name: " + name;
            }
        }
        
        return "Error: Field 'name' not found";
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        // HTML encode to prevent XSS
        return input.replaceAll("&", "&amp;")
                   .replaceAll("<", "&lt;")
                   .replaceAll(">", "&gt;")
                   .replaceAll("\\"", "&quot;")
                   .replaceAll("'", "&#x27;")
                   .replaceAll("/", "&#x2F;");
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        Map<String, String> payload1 = new HashMap<>();
        payload1.put("name", "John Doe");
        System.out.println(processPayload(payload1));
        
        System.out.println("\\nTest Case 2:");
        Map<String, String> payload2 = new HashMap<>();
        payload2.put("email", "test@example.com");
        System.out.println(processPayload(payload2));
        
        System.out.println("\\nTest Case 3:");
        Map<String, String> payload3 = new HashMap<>();
        payload3.put("name", "<script>alert('XSS')</script>");
        System.out.println(processPayload(payload3));
        
        System.out.println("\\nTest Case 4:");
        Map<String, String> payload4 = new HashMap<>();
        payload4.put("name", "Alice & Bob");
        System.out.println(processPayload(payload4));
        
        System.out.println("\\nTest Case 5:");
        System.out.println(processPayload(null));
    }
}
