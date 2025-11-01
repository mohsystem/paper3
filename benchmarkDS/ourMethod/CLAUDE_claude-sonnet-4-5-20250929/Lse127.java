
import java.io.*;
import java.nio.charset.StandardCharsets;

public class Lse127 {
    private static final int MAX_USERNAME_LENGTH = 100;
    private static final int MAX_REQUEST_SIZE = 1024;
    
    /**
     * Processes HTTP request and sends greeting response
     * @param request Input stream containing the client request
     * @param response Output stream for sending response to client
     * @throws IOException if I/O error occurs
     */
    public static void httpHelloPage(InputStream request, OutputStream response) throws IOException {
        if (request == null || response == null) {
            throw new IllegalArgumentException("Request and response streams cannot be null");
        }
        
        // Read request with size limit to prevent memory exhaustion
        byte[] buffer = new byte[MAX_REQUEST_SIZE];
        int bytesRead = 0;
        int totalRead = 0;
        
        // Read with bounds checking
        while (totalRead < MAX_REQUEST_SIZE && (bytesRead = request.read(buffer, totalRead, MAX_REQUEST_SIZE - totalRead)) != -1) {
            totalRead += bytesRead;
            if (totalRead >= MAX_REQUEST_SIZE) {
                break; // Prevent excessive memory usage
            }
        }
        
        // Convert to string using safe charset
        String requestStr = new String(buffer, 0, totalRead, StandardCharsets.UTF_8);
        
        // Extract username - validate and sanitize input
        String username = extractUsername(requestStr);
        
        // Validate username length
        if (username.length() > MAX_USERNAME_LENGTH) {
            username = username.substring(0, MAX_USERNAME_LENGTH);
        }
        
        // Sanitize username to prevent injection attacks
        username = sanitizeUsername(username);
        
        // Construct response with sanitized data
        String responseMessage = "Hello " + username;
        
        // Write response using safe charset encoding
        byte[] responseBytes = responseMessage.getBytes(StandardCharsets.UTF_8);
        response.write(responseBytes);
        response.flush();
    }
    
    /**
     * Extracts username from request string
     * @param request The request string
     * @return Extracted username or "Guest" if not found
     */
    private static String extractUsername(String request) {
        if (request == null || request.isEmpty()) {
            return "Guest";
        }
        
        // Simple extraction - look for username parameter
        // Format expected: username=value
        String[] parts = request.split("username=");
        if (parts.length < 2) {
            return "Guest";
        }
        
        // Get username portion and stop at first delimiter
        String usernamePart = parts[1];
        int endIndex = usernamePart.indexOf('&');
        if (endIndex == -1) {
            endIndex = usernamePart.indexOf('\\n');
        }
        if (endIndex == -1) {
            endIndex = usernamePart.indexOf('\\r');
        }
        if (endIndex == -1) {
            endIndex = usernamePart.length();
        }
        
        return usernamePart.substring(0, endIndex).trim();
    }
    
    /**
     * Sanitizes username to prevent injection attacks
     * Removes potentially dangerous characters
     * @param username The username to sanitize
     * @return Sanitized username
     */
    private static String sanitizeUsername(String username) {
        if (username == null || username.isEmpty()) {
            return "Guest";
        }
        
        // Allow only alphanumeric characters, spaces, hyphens, and underscores
        // Remove all other characters to prevent injection
        StringBuilder sanitized = new StringBuilder();
        for (char c : username.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == ' ' || c == '-' || c == '_') {
                sanitized.append(c);
            }
        }
        
        String result = sanitized.toString().trim();
        return result.isEmpty() ? "Guest" : result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal username
        try {
            ByteArrayInputStream req1 = new ByteArrayInputStream("username=Alice".getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream resp1 = new ByteArrayOutputStream();
            httpHelloPage(req1, resp1);
            System.out.println("Test 1: " + resp1.toString(StandardCharsets.UTF_8.name()));
        } catch (IOException e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Username with special characters (should be sanitized)
        try {
            ByteArrayInputStream req2 = new ByteArrayInputStream("username=Bob<script>".getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream resp2 = new ByteArrayOutputStream();
            httpHelloPage(req2, resp2);
            System.out.println("Test 2: " + resp2.toString(StandardCharsets.UTF_8.name()));
        } catch (IOException e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: No username provided
        try {
            ByteArrayInputStream req3 = new ByteArrayInputStream("other=data".getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream resp3 = new ByteArrayOutputStream();
            httpHelloPage(req3, resp3);
            System.out.println("Test 3: " + resp3.toString(StandardCharsets.UTF_8.name()));
        } catch (IOException e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Very long username (should be truncated)
        try {
            String longName = "username=" + "A".repeat(200);
            ByteArrayInputStream req4 = new ByteArrayInputStream(longName.getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream resp4 = new ByteArrayOutputStream();
            httpHelloPage(req4, resp4);
            System.out.println("Test 4: " + resp4.toString(StandardCharsets.UTF_8.name()));
        } catch (IOException e) {
            System.err.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Username with valid special characters
        try {
            ByteArrayInputStream req5 = new ByteArrayInputStream("username=John_Doe-123".getBytes(StandardCharsets.UTF_8));
            ByteArrayOutputStream resp5 = new ByteArrayOutputStream();
            httpHelloPage(req5, resp5);
            System.out.println("Test 5: " + resp5.toString(StandardCharsets.UTF_8.name()));
        } catch (IOException e) {
            System.err.println("Test 5 failed: " + e.getMessage());
        }
    }
}
