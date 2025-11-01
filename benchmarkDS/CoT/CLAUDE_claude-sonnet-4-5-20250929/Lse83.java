
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.json.JSONObject;
import org.json.JSONException;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse83 {
    private static final int MAX_PAYLOAD_SIZE = 1024 * 10; // 10KB limit
    private static final Pattern SAFE_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9\\\\s\\\\-_.]{1,100}$");

    public static String processPayload(String jsonPayload) {
        if (jsonPayload == null || jsonPayload.trim().isEmpty()) {
            return "{\\"error\\": \\"Empty payload\\"}";
        }

        if (jsonPayload.length() > MAX_PAYLOAD_SIZE) {
            return "{\\"error\\": \\"Payload too large\\"}";
        }

        try {
            JSONObject jsonObject = new JSONObject(jsonPayload);
            
            if (jsonObject.has("name")) {
                String name = jsonObject.getString("name");
                
                // Validate and sanitize the name field
                if (name == null || name.trim().isEmpty()) {
                    return "{\\"error\\": \\"Name field is empty\\"}";
                }
                
                if (!SAFE_NAME_PATTERN.matcher(name).matches()) {
                    return "{\\"error\\": \\"Invalid name format\\"}";
                }
                
                // Escape special characters to prevent XSS
                String sanitizedName = escapeJson(name);
                return "{\\"name\\": \\"" + sanitizedName + "\\"}";
            } else {
                return "{\\"error\\": \\"Name field not found\\"}";
            }
        } catch (JSONException e) {
            return "{\\"error\\": \\"Invalid JSON format\\"}";
        }
    }

    private static String escapeJson(String input) {
        return input.replace("\\\\", "\\\\\\\\")\n                   .replace("\\"", "\\\\\\"")
                   .replace("\\n", "\\\\n")
                   .replace("\\r", "\\\\r")
                   .replace("\\t", "\\\\t");
    }

    static class PayloadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                InputStream is = exchange.getRequestBody();
                String payload = new String(is.readAllBytes(), StandardCharsets.UTF_8);
                
                String response = processPayload(payload);
                
                exchange.getResponseHeaders().add("Content-Type", "application/json");
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
            } else {
                String response = "{\\"error\\": \\"Method not allowed\\"}";
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
            }
        }
    }

    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Case 1: " + processPayload("{\\"name\\": \\"John Doe\\"}"));
        System.out.println("Test Case 2: " + processPayload("{\\"age\\": 30}"));
        System.out.println("Test Case 3: " + processPayload("{\\"name\\": \\"\\"}"));
        System.out.println("Test Case 4: " + processPayload("{}"));
        System.out.println("Test Case 5: " + processPayload("{\\"name\\": \\"Alice_123\\"}"));

        // Start web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/api/payload", new PayloadHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
    }
}
