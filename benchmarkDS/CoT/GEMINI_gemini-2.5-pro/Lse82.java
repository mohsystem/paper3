import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import org.json.JSONException;
import org.json.JSONObject;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

// Note: This code uses the org.json library. 
// You would need to add the dependency (e.g., via Maven/Gradle) or have the JAR in your classpath.
// For a single file compilation, you might need to download the jar and compile like:
// javac -cp .:/path/to/json.jar Lse82.java
// java -cp .:/path/to/json.jar Lse82
public class Lse82 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("1. Success: curl 'http://localhost:8080/api?payload=%7B%22name%22%3A%22Alice%22%2C%22age%22%3A30%7D'");
        System.out.println("   Expected: Alice");
        System.out.println("2. Missing payload parameter: curl 'http://localhost:8080/api'");
        System.out.println("   Expected: 400 Bad Request - Payload parameter is missing");
        System.out.println("3. Invalid JSON in payload: curl 'http://localhost:8080/api?payload=invalid-json'");
        System.out.println("   Expected: 400 Bad Request - Invalid JSON format in payload");
        System.out.println("4. Missing 'name' key in payload: curl 'http://localhost:8080/api?payload=%7B%22age%22%3A30%7D'");
        System.out.println("   Expected: 400 Bad Request - 'name' key is missing or not a string");
        System.out.println("5. Payload is not a JSON object: curl 'http://localhost:8080/api?payload=%5B%22test%22%5D'");
        System.out.println("   Expected: 400 Bad Request - Invalid JSON format in payload");
    }

    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                String query = exchange.getRequestURI().getQuery();
                Map<String, String> params = queryToMap(query);
                String payload = params.get("payload");

                if (payload == null) {
                    sendResponse(exchange, 400, "Payload parameter is missing");
                    return;
                }
                
                try {
                    // It's good practice to URL-decode the payload
                    String decodedPayload = URLDecoder.decode(payload, StandardCharsets.UTF_8.name());
                    JSONObject json = new JSONObject(decodedPayload);
                    
                    // Use optString to avoid JSONException if key doesn't exist
                    String name = json.optString("name", null);

                    if (name == null) {
                        sendResponse(exchange, 400, "'name' key is missing or not a string");
                    } else {
                        sendResponse(exchange, 200, name);
                    }
                } catch (JSONException e) {
                    sendResponse(exchange, 400, "Invalid JSON format in payload");
                } catch (Exception e) {
                    sendResponse(exchange, 500, "Internal Server Error");
                }
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }

        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null) {
                return result;
            }
            for (String param : query.split("&")) {
                String[] entry = param.split("=");
                if (entry.length > 1) {
                    result.put(entry[0], entry[1]);
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "text/plain");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }
}