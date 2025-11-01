
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse128 {
    private static final int MAX_USERNAME_LENGTH = 100;
    private static final Pattern VALID_USERNAME = Pattern.compile("^[a-zA-Z0-9_-]+$");
    
    /**
     * Sanitize and validate username input
     * @param username Raw username from request
     * @return Sanitized username or null if invalid
     */
    private static String sanitizeUsername(String username) {
        if (username == null || username.isEmpty()) {
            return null;
        }
        
        // Trim and limit length to prevent buffer issues
        username = username.trim();
        if (username.length() > MAX_USERNAME_LENGTH) {
            return null;
        }
        
        // Only allow alphanumeric, underscore, and hyphen
        if (!VALID_USERNAME.matcher(username).matches()) {
            return null;
        }
        
        return username;
    }
    
    /**
     * HTML encode the username to prevent XSS
     * @param input Raw input string
     * @return HTML-encoded string
     */
    private static String htmlEncode(String input) {
        if (input == null) {
            return "";
        }
        
        StringBuilder encoded = new StringBuilder(input.length() * 2);
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<':
                    encoded.append("&lt;");
                    break;
                case '>':
                    encoded.append("&gt;");
                    break;
                case '&':
                    encoded.append("&amp;");
                    break;
                case '"':
                    encoded.append("&quot;");
                    break;
                case '\\'':
                    encoded.append("&#x27;");
                    break;
                default:
                    encoded.append(c);
            }
        }
        return encoded.toString();
    }
    
    /**
     * Process HTTP request and generate hello page response
     * @param request Input stream containing the HTTP request
     * @param response Output stream for the HTTP response
     */
    public static void httpHelloPage(InputStream request, OutputStream response) {
        BufferedReader reader = null;
        BufferedWriter writer = null;
        
        try {
            // Wrap streams with buffered readers/writers using UTF-8
            reader = new BufferedReader(
                new InputStreamReader(request, StandardCharsets.UTF_8)
            );
            writer = new BufferedWriter(
                new OutputStreamWriter(response, StandardCharsets.UTF_8)
            );
            
            String username = null;
            String line;
            
            // Read request headers and extract username parameter
            // Limit total bytes read to prevent DoS
            int bytesRead = 0;
            final int MAX_REQUEST_SIZE = 8192;
            
            while ((line = reader.readLine()) != null && bytesRead < MAX_REQUEST_SIZE) {
                bytesRead += line.length() + 2; // +2 for CRLF
                
                // Empty line indicates end of headers
                if (line.isEmpty()) {
                    break;
                }
                
                // Look for username in query string of GET request
                if (line.startsWith("GET ")) {
                    String[] parts = line.split("\\\\s+");
                    if (parts.length >= 2) {
                        String path = parts[1];
                        int queryStart = path.indexOf('?');
                        if (queryStart != -1) {
                            String query = path.substring(queryStart + 1);
                            String[] params = query.split("&");
                            for (String param : params) {
                                String[] kv = param.split("=", 2);
                                if (kv.length == 2 && "username".equals(kv[0])) {
                                    // URL decode username (basic implementation)
                                    username = java.net.URLDecoder.decode(kv[1], "UTF-8");
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            // Validate and sanitize username
            String sanitizedUsername = sanitizeUsername(username);
            
            // Construct response
            if (sanitizedUsername != null) {
                // HTML encode for output to prevent XSS
                String encodedUsername = htmlEncode(sanitizedUsername);
                
                // Write HTTP response with security headers
                writer.write("HTTP/1.1 200 OK\\r\\n");
                writer.write("Content-Type: text/html; charset=UTF-8\\r\\n");
                writer.write("X-Content-Type-Options: nosniff\\r\\n");
                writer.write("X-Frame-Options: DENY\\r\\n");
                writer.write("Cache-Control: no-store\\r\\n");
                writer.write("\\r\\n");
                writer.write("<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"></head><body>");
                writer.write("hello " + encodedUsername);
                writer.write("</body></html>");
            } else {
                // Return generic error message
                writer.write("HTTP/1.1 400 Bad Request\\r\\n");
                writer.write("Content-Type: text/plain; charset=UTF-8\\r\\n");
                writer.write("\\r\\n");
                writer.write("Invalid username");
            }
            
            writer.flush();
            
        } catch (IOException e) {
            // Log error securely without exposing details
            System.err.println("Error processing request");
        } finally {
            // Clean up resources
            try {
                if (writer != null) {
                    writer.close();
                }
            } catch (IOException e) {
                // Ignore close errors
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid username
        testCase("GET /?username=john HTTP/1.1\\r\\n\\r\\n", "Valid username");
        
        // Test case 2: Username with special chars (should be rejected)
        testCase("GET /?username=john<script> HTTP/1.1\\r\\n\\r\\n", "XSS attempt");
        
        // Test case 3: Long username (should be rejected)
        testCase("GET /?username=" + "a".repeat(150) + " HTTP/1.1\\r\\n\\r\\n", "Long username");
        
        // Test case 4: Valid username with URL encoding
        testCase("GET /?username=john_doe HTTP/1.1\\r\\n\\r\\n", "Username with underscore");
        
        // Test case 5: No username provided
        testCase("GET / HTTP/1.1\\r\\n\\r\\n", "No username");
    }
    
    private static void testCase(String request, String description) {
        System.out.println("\\n=== Test: " + description + " ===");
        ByteArrayInputStream requestStream = new ByteArrayInputStream(
            request.getBytes(StandardCharsets.UTF_8)
        );
        ByteArrayOutputStream responseStream = new ByteArrayOutputStream();
        
        httpHelloPage(requestStream, responseStream);
        
        System.out.println(new String(responseStream.toByteArray(), StandardCharsets.UTF_8));
    }
}
