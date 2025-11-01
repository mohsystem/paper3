
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import org.json.JSONObject;
import org.json.JSONException;

public class Lse82 {
    // Maximum allowed size for payload to prevent DoS
    private static final int MAX_PAYLOAD_SIZE = 10240; // 10KB limit
    
    public static String processPayload(String payload) {
        // Security: Validate input is not null and within size limits
        if (payload == null || payload.isEmpty()) {
            return "{\\"error\\":\\"Invalid input\\"}";
        }
        
        if (payload.length() > MAX_PAYLOAD_SIZE) {
            return "{\\"error\\":\\"Payload too large\\"}";
        }
        
        try {
            // Security: Use safe JSON parsing without evaluating code
            // Parse the payload as JSON into a dictionary-like structure
            JSONObject jsonDict = new JSONObject(payload);
            
            // Security: Validate that 'name' key exists before accessing
            if (!jsonDict.has("name")) {
                return "{\\"error\\":\\"Missing name field\\"}";
            }
            
            // Security: Get the name value safely, treating as untrusted data
            String name = jsonDict.getString("name");
            
            // Security: Sanitize output to prevent injection attacks
            // Escape special characters for JSON output
            name = sanitizeForJson(name);
            
            return "{\\"name\\":\\"" + name + "\\"}";
            
        } catch (JSONException e) {
            // Security: Generic error message, no internal details leaked
            return "{\\"error\\":\\"Invalid JSON format\\"}";
        }
    }
    
    // Security: Sanitize string for safe JSON output
    private static String sanitizeForJson(String input) {
        if (input == null) {
            return "";
        }
        // Escape special JSON characters to prevent injection
        return input.replace("\\\\", "\\\\\\\\")\n                    .replace("\\"", "\\\\\\"")
                    .replace("\\n", "\\\\n")
                    .replace("\\r", "\\\\r")
                    .replace("\\t", "\\\\t");
    }
    
    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Security: Only accept GET requests for this endpoint
            if (!"GET".equals(exchange.getRequestMethod())) {
                String response = "{\\"error\\":\\"Method not allowed\\"}";
                exchange.getResponseHeaders().set("Content-Type", "application/json");
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
                return;
            }
            
            // Security: Parse query parameters safely
            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = parseQuery(query);
            
            String payload = params.get("payload");
            String result = processPayload(payload);
            
            // Security: Set secure response headers
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.sendResponseHeaders(200, result.length());
            
            OutputStream os = exchange.getResponseBody();
            os.write(result.getBytes(StandardCharsets.UTF_8));
            os.close();
        }
        
        // Security: Safe query parameter parsing with validation
        private Map<String, String> parseQuery(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null || query.isEmpty()) {
                return result;
            }
            
            String[] pairs = query.split("&");
            for (String pair : pairs) {
                int idx = pair.indexOf("=");
                if (idx > 0 && idx < pair.length() - 1) {
                    try {
                        // Security: URL decode with explicit charset
                        String key = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8.name());
                        String value = URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8.name());
                        result.put(key, value);
                    } catch (Exception e) {
                        // Security: Skip malformed parameters, continue processing
                    }
                }
            }
            return result;
        }
    }
    
    public static void main(String[] args) {
        try {
            // Create HTTP server on port 8000
            HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
            server.createContext("/api", new ApiHandler());
            server.setExecutor(null);
            server.start();
            System.out.println("Server started on port 8000");
            
            // Test cases
            System.out.println("\\nTest cases:");
            System.out.println("1. Valid JSON: " + processPayload("{\\"name\\":\\"John\\"}"));
            System.out.println("2. Valid JSON with special chars: " + processPayload("{\\"name\\":\\"O'Brien\\"}"));
            System.out.println("3. Missing name field: " + processPayload("{\\"age\\":30}"));
            System.out.println("4. Invalid JSON: " + processPayload("{invalid}"));
            System.out.println("5. Null payload: " + processPayload(null));
            
        } catch (IOException e) {
            System.err.println("Failed to start server");
        }
    }
}
