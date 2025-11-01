
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import com.sun.net.httpserver.*;

public class Lse126 {
    // Maximum allowed request body size to prevent resource exhaustion
    private static final int MAX_REQUEST_BODY_SIZE = 8192; // 8KB
    // Maximum username length to prevent buffer issues
    private static final int MAX_USERNAME_LENGTH = 256;
    
    public static void httpHelloPage(HttpExchange exchange) throws IOException {
        // Only accept POST requests for this endpoint
        if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
            sendError(exchange, 405, "Method Not Allowed");
            return;
        }
        
        String username = null;
        
        try (InputStream is = exchange.getRequestBody()) {
            // Read request body with size limit to prevent DoS
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] chunk = new byte[1024];
            int bytesRead;
            int totalRead = 0;
            
            while ((bytesRead = is.read(chunk)) != -1) {
                totalRead += bytesRead;
                // Enforce maximum request body size
                if (totalRead > MAX_REQUEST_BODY_SIZE) {
                    sendError(exchange, 413, "Request Entity Too Large");
                    return;
                }
                buffer.write(chunk, 0, bytesRead);
            }
            
            // Decode username from request body with proper character encoding
            username = buffer.toString(StandardCharsets.UTF_8.name());
            
            // Validate username: not null, not empty, within length limits
            if (username == null || username.isEmpty()) {
                sendError(exchange, 400, "Invalid Request");
                return;
            }
            
            // Trim and enforce maximum username length
            username = username.trim();
            if (username.length() > MAX_USERNAME_LENGTH) {
                sendError(exchange, 400, "Invalid Request");
                return;
            }
            
            // Sanitize username to prevent XSS attacks
            username = sanitizeHtml(username);
            
            // Construct response HTML with proper escaping
            StringBuilder outputbuf = new StringBuilder();
            outputbuf.append("<!DOCTYPE html>\\n");
            outputbuf.append("<html lang=\\"en\\">\\n");
            outputbuf.append("<head>\\n");
            outputbuf.append("<meta charset=\\"UTF-8\\">\\n");
            outputbuf.append("<meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n");
            outputbuf.append("<title>Hello Page</title>\\n");
            outputbuf.append("</head>\\n");
            outputbuf.append("<body>\\n");
            outputbuf.append("<h1>Hello, ").append(username).append("!</h1>\\n");
            outputbuf.append("</body>\\n");
            outputbuf.append("</html>");
            
            // Convert response to bytes with explicit encoding
            byte[] responseBytes = outputbuf.toString().getBytes(StandardCharsets.UTF_8);
            
            // Set response headers
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            // Security headers to prevent common attacks
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
            exchange.getResponseHeaders().set("Content-Security-Policy", "default-src 'self'");
            
            // Send response with proper status code and content length
            exchange.sendResponseHeaders(200, responseBytes.length);
            
            // Write response body
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
                os.flush();
            }
            
        } catch (IOException e) {
            // Log error securely without exposing internal details
            System.err.println("Error processing request");
            sendError(exchange, 500, "Internal Server Error");
        } finally {
            // Ensure response stream is closed
            exchange.close();
        }
    }
    
    // HTML entity encoding to prevent XSS
    private static String sanitizeHtml(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\\"", "&quot;")
                    .replace("'", "&#x27;")
                    .replace("/", "&#x2F;");
    }
    
    private static void sendError(HttpExchange exchange, int statusCode, String message) {
        try {
            byte[] response = message.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response);
            }
        } catch (IOException e) {
            System.err.println("Error sending error response");
        } finally {
            exchange.close();
        }
    }
    
    public static void main(String[] args) {
        try {
            // Create HTTP server on port 8080
            HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
            server.createContext("/hello", Lse126::httpHelloPage);
            server.setExecutor(null);
            server.start();
            System.out.println("Server started on port 8080");
            
            // Test cases
            testHttpHelloPage("Alice");
            testHttpHelloPage("Bob123");
            testHttpHelloPage("<script>alert('xss')</script>");
            testHttpHelloPage("A".repeat(300)); // Long username
            testHttpHelloPage(""); // Empty username
            
        } catch (IOException e) {
            System.err.println("Failed to start server");
        }
    }
    
    private static void testHttpHelloPage(String username) {
        try {
            URL url = new URL("http://localhost:8080/hello");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("POST");
            conn.setDoOutput(true);
            conn.setRequestProperty("Content-Type", "text/plain; charset=UTF-8");
            
            try (OutputStream os = conn.getOutputStream()) {
                os.write(username.getBytes(StandardCharsets.UTF_8));
            }
            
            int responseCode = conn.getResponseCode();
            System.out.println("Test username: " + username.substring(0, Math.min(username.length(), 50)));
            System.out.println("Response code: " + responseCode);
            
            try (BufferedReader br = new BufferedReader(
                    new InputStreamReader(conn.getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                while ((line = br.readLine()) != null) {
                    System.out.println(line);
                }
            }
            System.out.println();
            
        } catch (Exception e) {
            System.err.println("Test failed: " + e.getMessage());
        }
    }
}
