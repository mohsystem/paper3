
import java.io.*;
import java.net.*;
import java.util.*;

public class Task130 {
    
    // CORS configuration
    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
        "https://example.com",
        "https://app.example.com",
        "http://localhost:3000"
    ));
    
    private static final Set<String> ALLOWED_METHODS = new HashSet<>(Arrays.asList(
        "GET", "POST", "PUT", "DELETE", "OPTIONS"
    ));
    
    private static final Set<String> ALLOWED_HEADERS = new HashSet<>(Arrays.asList(
        "Content-Type", "Authorization", "X-Requested-With"
    ));
    
    private static final int MAX_AGE = 3600; // 1 hour
    
    // Simple API Response class
    static class APIResponse {
        private int statusCode;
        private Map<String, String> headers;
        private String body;
        
        public APIResponse(int statusCode) {
            this.statusCode = statusCode;
            this.headers = new HashMap<>();
            this.body = "";
        }
        
        public void setHeader(String key, String value) {
            headers.put(key, value);
        }
        
        public void setBody(String body) {
            this.body = body;
        }
        
        public int getStatusCode() {
            return statusCode;
        }
        
        public Map<String, String> getHeaders() {
            return headers;
        }
        
        public String getBody() {
            return body;
        }
        
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("Status Code: ").append(statusCode).append("\\n");
            sb.append("Headers:\\n");
            for (Map.Entry<String, String> entry : headers.entrySet()) {
                sb.append("  ").append(entry.getKey()).append(": ").append(entry.getValue()).append("\\n");
            }
            sb.append("Body: ").append(body);
            return sb.toString();
        }
    }
    
    // Handle CORS for incoming request
    public static APIResponse handleCORS(String origin, String method, String requestHeaders) {
        APIResponse response = new APIResponse(200);
        
        // Check if origin is allowed
        if (origin != null && ALLOWED_ORIGINS.contains(origin)) {
            response.setHeader("Access-Control-Allow-Origin", origin);
            response.setHeader("Access-Control-Allow-Credentials", "true");
            response.setHeader("Vary", "Origin");
        } else if (origin != null) {
            // Origin not allowed - return 403
            response = new APIResponse(403);
            response.setBody("{\\"error\\": \\"Origin not allowed\\"}");
            return response;
        }
        
        // Handle preflight OPTIONS request
        if ("OPTIONS".equals(method)) {
            response.setHeader("Access-Control-Allow-Methods", String.join(", ", ALLOWED_METHODS));
            response.setHeader("Access-Control-Allow-Headers", String.join(", ", ALLOWED_HEADERS));
            response.setHeader("Access-Control-Max-Age", String.valueOf(MAX_AGE));
            response.setStatusCode(204);
        } else {
            // Check if method is allowed
            if (!ALLOWED_METHODS.contains(method)) {
                response = new APIResponse(405);
                response.setBody("{\\"error\\": \\"Method not allowed\\"}");
                return response;
            }
            
            // Expose headers that client can access
            response.setHeader("Access-Control-Expose-Headers", "Content-Length, X-Request-ID");
        }
        
        return response;
    }
    
    // Simulate API endpoint
    public static APIResponse handleAPIRequest(String origin, String method, String path) {
        // First handle CORS
        APIResponse corsResponse = handleCORS(origin, method, null);
        
        // If CORS check failed or it's a preflight, return early
        if (corsResponse.getStatusCode() != 200 && corsResponse.getStatusCode() != 204) {
            return corsResponse;
        }
        
        // Handle actual API request
        if ("GET".equals(method) && "/api/data".equals(path)) {
            corsResponse.setBody("{\\"message\\": \\"Data retrieved successfully\\", \\"data\\": [1, 2, 3]}");
        } else if ("POST".equals(method) && "/api/data".equals(path)) {
            corsResponse.setBody("{\\"message\\": \\"Data created successfully\\", \\"id\\": 123}");
        } else {
            corsResponse = new APIResponse(404);
            corsResponse.setBody("{\\"error\\": \\"Endpoint not found\\"}");
        }
        
        corsResponse.setHeader("Content-Type", "application/json");
        return corsResponse;
    }
    
    public static void main(String[] args) {
        System.out.println("=== CORS API Implementation Test Cases ===\\n");
        
        // Test Case 1: Preflight request from allowed origin
        System.out.println("Test Case 1: Preflight OPTIONS request from allowed origin");
        APIResponse response1 = handleAPIRequest("https://example.com", "OPTIONS", "/api/data");
        System.out.println(response1);
        System.out.println("\\n" + "=".repeat(60) + "\\n");
        
        // Test Case 2: GET request from allowed origin
        System.out.println("Test Case 2: GET request from allowed origin");
        APIResponse response2 = handleAPIRequest("https://example.com", "GET", "/api/data");
        System.out.println(response2);
        System.out.println("\\n" + "=".repeat(60) + "\\n");
        
        // Test Case 3: POST request from allowed origin
        System.out.println("Test Case 3: POST request from allowed origin");
        APIResponse response3 = handleAPIRequest("http://localhost:3000", "POST", "/api/data");
        System.out.println(response3);
        System.out.println("\\n" + "=".repeat(60) + "\\n");
        
        // Test Case 4: Request from disallowed origin
        System.out.println("Test Case 4: Request from disallowed origin");
        APIResponse response4 = handleAPIRequest("https://malicious.com", "GET", "/api/data");
        System.out.println(response4);
        System.out.println("\\n" + "=".repeat(60) + "\\n");
        
        // Test Case 5: Request with disallowed method
        System.out.println("Test Case 5: Request with disallowed method");
        APIResponse response5 = handleAPIRequest("https://example.com", "PATCH", "/api/data");
        System.out.println(response5);
        System.out.println("\\n" + "=".repeat(60) + "\\n");
    }
}
