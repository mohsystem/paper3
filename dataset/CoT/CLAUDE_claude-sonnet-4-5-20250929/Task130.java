
import java.io.*;
import java.net.*;
import java.util.*;

public class Task130 {
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
    
    private static final int MAX_AGE = 3600;

    public static class CORSResponse {
        public boolean allowed;
        public Map<String, String> headers;
        public String message;

        public CORSResponse(boolean allowed, Map<String, String> headers, String message) {
            this.allowed = allowed;
            this.headers = headers;
            this.message = message;
        }
    }

    public static CORSResponse handleCORS(String origin, String method, String requestHeaders) {
        Map<String, String> responseHeaders = new HashMap<>();
        
        // Validate and sanitize origin
        String sanitizedOrigin = sanitizeHeader(origin);
        if (sanitizedOrigin == null || sanitizedOrigin.isEmpty()) {
            return new CORSResponse(false, responseHeaders, "Invalid origin");
        }

        // Check if origin is allowed
        if (!ALLOWED_ORIGINS.contains(sanitizedOrigin)) {
            return new CORSResponse(false, responseHeaders, "Origin not allowed");
        }

        // Validate method
        String sanitizedMethod = sanitizeHeader(method);
        if (sanitizedMethod == null || !ALLOWED_METHODS.contains(sanitizedMethod.toUpperCase())) {
            return new CORSResponse(false, responseHeaders, "Method not allowed");
        }

        // Set CORS headers
        responseHeaders.put("Access-Control-Allow-Origin", sanitizedOrigin);
        responseHeaders.put("Access-Control-Allow-Methods", String.join(", ", ALLOWED_METHODS));
        responseHeaders.put("Access-Control-Allow-Headers", String.join(", ", ALLOWED_HEADERS));
        responseHeaders.put("Access-Control-Max-Age", String.valueOf(MAX_AGE));
        responseHeaders.put("Access-Control-Allow-Credentials", "true");
        
        // Security headers
        responseHeaders.put("X-Content-Type-Options", "nosniff");
        responseHeaders.put("X-Frame-Options", "DENY");
        responseHeaders.put("Strict-Transport-Security", "max-age=31536000; includeSubDomains");

        // Handle preflight request
        if ("OPTIONS".equalsIgnoreCase(sanitizedMethod)) {
            if (requestHeaders != null && !requestHeaders.isEmpty()) {
                String sanitizedHeaders = sanitizeHeader(requestHeaders);
                if (sanitizedHeaders != null && validateRequestedHeaders(sanitizedHeaders)) {
                    return new CORSResponse(true, responseHeaders, "Preflight request approved");
                }
            }
            return new CORSResponse(true, responseHeaders, "Preflight request approved");
        }

        return new CORSResponse(true, responseHeaders, "CORS request approved");
    }

    private static String sanitizeHeader(String header) {
        if (header == null) {
            return null;
        }
        
        // Remove any control characters and trim
        String sanitized = header.replaceAll("[\\\\x00-\\\\x1F\\\\x7F]", "").trim();
        
        // Check for header injection attempts
        if (sanitized.contains("\\r") || sanitized.contains("\\n")) {
            return null;
        }
        
        return sanitized;
    }

    private static boolean validateRequestedHeaders(String requestedHeaders) {
        String[] headers = requestedHeaders.split(",");
        for (String header : headers) {
            String trimmed = header.trim().toLowerCase();
            boolean found = false;
            for (String allowed : ALLOWED_HEADERS) {
                if (allowed.toLowerCase().equals(trimmed)) {
                    found = true;
                    break;
                }
            }
            if (!found && !trimmed.isEmpty()) {
                return false;
            }
        }
        return true;
    }

    public static void printResponse(CORSResponse response) {
        System.out.println("Allowed: " + response.allowed);
        System.out.println("Message: " + response.message);
        System.out.println("Headers:");
        for (Map.Entry<String, String> entry : response.headers.entrySet()) {
            System.out.println("  " + entry.getKey() + ": " + entry.getValue());
        }
        System.out.println();
    }

    public static void main(String[] args) {
        System.out.println("=== CORS Implementation Test Cases ===\\n");

        // Test case 1: Valid GET request
        System.out.println("Test 1: Valid GET request from allowed origin");
        CORSResponse resp1 = handleCORS("https://example.com", "GET", null);
        printResponse(resp1);

        // Test case 2: Valid OPTIONS preflight request
        System.out.println("Test 2: Valid OPTIONS preflight request");
        CORSResponse resp2 = handleCORS("https://app.example.com", "OPTIONS", "Content-Type, Authorization");
        printResponse(resp2);

        // Test case 3: Invalid origin
        System.out.println("Test 3: Request from non-allowed origin");
        CORSResponse resp3 = handleCORS("https://malicious.com", "GET", null);
        printResponse(resp3);

        // Test case 4: Invalid method
        System.out.println("Test 4: Request with non-allowed method");
        CORSResponse resp4 = handleCORS("https://example.com", "TRACE", null);
        printResponse(resp4);

        // Test case 5: Valid POST request from localhost
        System.out.println("Test 5: Valid POST request from localhost");
        CORSResponse resp5 = handleCORS("http://localhost:3000", "POST", "Content-Type");
        printResponse(resp5);
    }
}
