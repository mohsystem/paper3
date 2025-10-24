
import java.io.*;
import java.net.*;
import java.util.*;

public class Task130 {
    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
        "http://localhost:3000",
        "http://example.com",
        "https://example.com"
    ));
    
    private static final Set<String> ALLOWED_METHODS = new HashSet<>(Arrays.asList(
        "GET", "POST", "PUT", "DELETE", "OPTIONS"
    ));
    
    private static final Set<String> ALLOWED_HEADERS = new HashSet<>(Arrays.asList(
        "Content-Type", "Authorization", "X-Requested-With"
    ));
    
    public static Map<String, String> handleCORS(String origin, String method, String requestHeaders) {
        Map<String, String> headers = new HashMap<>();
        
        if (origin != null && ALLOWED_ORIGINS.contains(origin)) {
            headers.put("Access-Control-Allow-Origin", origin);
        }
        
        headers.put("Access-Control-Allow-Methods", String.join(", ", ALLOWED_METHODS));
        headers.put("Access-Control-Allow-Headers", String.join(", ", ALLOWED_HEADERS));
        headers.put("Access-Control-Max-Age", "3600");
        headers.put("Access-Control-Allow-Credentials", "true");
        
        return headers;
    }
    
    public static String handleAPIRequest(String origin, String method, String requestHeaders, String path) {
        Map<String, String> corsHeaders = handleCORS(origin, method, requestHeaders);
        
        if ("OPTIONS".equals(method)) {
            return formatResponse(204, "No Content", corsHeaders, "");
        }
        
        if (origin != null && !ALLOWED_ORIGINS.contains(origin)) {
            return formatResponse(403, "Forbidden", corsHeaders, "{\\"error\\": \\"Origin not allowed\\"}");
        }
        
        if (!ALLOWED_METHODS.contains(method)) {
            return formatResponse(405, "Method Not Allowed", corsHeaders, "{\\"error\\": \\"Method not allowed\\"}");
        }
        
        String responseBody = "";
        switch (path) {
            case "/api/users":
                responseBody = "{\\"users\\": [{\\"id\\": 1, \\"name\\": \\"John\\"}, {\\"id\\": 2, \\"name\\": \\"Jane\\"}]}";
                break;
            case "/api/data":
                responseBody = "{\\"data\\": \\"Sample data from API\\"}";
                break;
            default:
                responseBody = "{\\"message\\": \\"API endpoint not found\\"}";
        }
        
        return formatResponse(200, "OK", corsHeaders, responseBody);
    }
    
    private static String formatResponse(int statusCode, String statusText, Map<String, String> headers, String body) {
        StringBuilder response = new StringBuilder();
        response.append("HTTP/1.1 ").append(statusCode).append(" ").append(statusText).append("\\n");
        
        for (Map.Entry<String, String> header : headers.entrySet()) {
            response.append(header.getKey()).append(": ").append(header.getValue()).append("\\n");
        }
        
        response.append("Content-Type: application/json\\n");
        response.append("Content-Length: ").append(body.length()).append("\\n\\n");
        response.append(body);
        
        return response.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid GET request from allowed origin");
        System.out.println(handleAPIRequest("http://localhost:3000", "GET", "Content-Type", "/api/users"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 2: OPTIONS preflight request");
        System.out.println(handleAPIRequest("http://example.com", "OPTIONS", "Content-Type", "/api/data"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 3: Request from disallowed origin");
        System.out.println(handleAPIRequest("http://malicious.com", "GET", "Content-Type", "/api/users"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 4: Invalid HTTP method");
        System.out.println(handleAPIRequest("http://localhost:3000", "TRACE", "Content-Type", "/api/data"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 5: POST request to valid endpoint");
        System.out.println(handleAPIRequest("https://example.com", "POST", "Content-Type, Authorization", "/api/data"));
    }
}
