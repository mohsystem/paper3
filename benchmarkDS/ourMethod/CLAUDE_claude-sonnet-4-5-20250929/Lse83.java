
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import org.json.JSONObject;
import org.json.JSONException;

public class Lse83 {
    private static final int MAX_PAYLOAD_SIZE = 1024 * 10; // 10KB limit to prevent resource exhaustion

    // Validates and extracts the 'name' field from JSON payload
    public static String processPayload(String payload) {
        // Input validation: check payload is not null or empty
        if (payload == null || payload.trim().isEmpty()) {
            return "Error: Empty payload";
        }

        // Input validation: check payload size to prevent DoS
        if (payload.length() > MAX_PAYLOAD_SIZE) {
            return "Error: Payload too large";
        }

        try {
            // Parse JSON with explicit UTF-8 encoding
            JSONObject jsonObject = new JSONObject(payload);
            
            // Check if 'name' field exists
            if (jsonObject.has("name")) {
                // Extract name field value
                Object nameValue = jsonObject.get("name");
                
                // Validate that name is a string type
                if (nameValue instanceof String) {
                    String name = (String) nameValue;
                    
                    // Input validation: check name length (max 100 chars)
                    if (name.length() > 100) {
                        return "Error: Name too long";
                    }
                    
                    // Input validation: sanitize by allowing only alphanumeric, space, and basic punctuation
                    if (!name.matches("^[a-zA-Z0-9 ._-]*$")) {
                        return "Error: Invalid characters in name";
                    }
                    
                    // Return sanitized name content
                    return name;
                } else {
                    return "Error: Name must be a string";
                }
            } else {
                return "Error: Name field not found";
            }
        } catch (JSONException e) {
            // Log error internally, return generic message to user
            System.err.println("JSON parsing error: " + e.getMessage());
            return "Error: Invalid JSON format";
        }
    }

    static class PayloadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Only accept POST requests
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                String response = "Error: Only POST method allowed";
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
                return;
            }

            // Read request body with size limit
            InputStream is = exchange.getRequestBody();
            StringBuilder payload = new StringBuilder();
            try (BufferedReader br = new BufferedReader(
                    new InputStreamReader(is, StandardCharsets.UTF_8))) {
                String line;
                int totalSize = 0;
                while ((line = br.readLine()) != null) {
                    totalSize += line.length();
                    if (totalSize > MAX_PAYLOAD_SIZE) {
                        String response = "Error: Payload too large";
                        exchange.sendResponseHeaders(413, response.length());
                        OutputStream os = exchange.getResponseBody();
                        os.write(response.getBytes(StandardCharsets.UTF_8));
                        os.close();
                        return;
                    }
                    payload.append(line);
                }
            }

            // Process payload
            String result = processPayload(payload.toString());
            
            // Set response headers with security considerations
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            
            // Send response
            byte[] responseBytes = result.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(200, responseBytes.length);
            OutputStream os = exchange.getResponseBody();
            os.write(responseBytes);
            os.close();
        }
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 - Valid name:");
        System.out.println(processPayload("{\\"name\\":\\"John_Doe\\"}"));
        
        System.out.println("\\nTest Case 2 - Missing name field:");
        System.out.println(processPayload("{\\"age\\":30}"));
        
        System.out.println("\\nTest Case 3 - Invalid JSON:");
        System.out.println(processPayload("{invalid json}"));
        
        System.out.println("\\nTest Case 4 - Name with invalid characters:");
        System.out.println(processPayload("{\\"name\\":\\"<script>alert('xss')</script>\\"}"));
        
        System.out.println("\\nTest Case 5 - Empty payload:");
        System.out.println(processPayload(""));

        // Uncomment to start web server
        // try {
        //     HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        //     server.createContext("/payload", new PayloadHandler());
        //     server.setExecutor(null);
        //     server.start();
        //     System.out.println("Server started on port 8000");
        // } catch (IOException e) {
        //     System.err.println("Server error: " + e.getMessage());
        // }
    }
}
