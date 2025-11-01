/*
 * This Java code creates a simple web server that listens on port 8080.
 * It requires the 'org.json' library.
 *
 * How to compile and run:
 * 1. Make sure you have a JDK installed.
 * 2. Download the 'org.json' library JAR file (e.g., from Maven Central).
 *    https://repo1.maven.org/maven2/org/json/json/20231013/json-20231013.jar
 * 3. Compile the code with the JAR in the classpath:
 *    javac -cp .;<path_to_json.jar> Lse82.java
 *    (on Linux/macOS, use ':' instead of ';': javac -cp .:<path_to_json.jar> Lse82.java)
 * 4. Run the compiled code:
 *    java -cp .;<path_to_json.jar> Lse82
 *    (on Linux/macOS, use ':' instead of ';': java -cp .:<path_to_json.jar> Lse82)
 * 5. The server will start on http://localhost:8080.
 */
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import org.json.JSONObject;
import org.json.JSONException;

import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class Lse82 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Server started on port " + port);
        System.out.println("Use curl to test the API endpoint:");
        // Test cases (to be executed from a separate terminal):
        // 1. Valid request:
        System.out.println("curl 'http://localhost:8080/api?payload=%7B%22name%22%3A%22world%22%7D'");
        // 2. Missing 'payload' parameter:
        System.out.println("curl 'http://localhost:8080/api'");
        // 3. Invalid JSON in 'payload':
        System.out.println("curl 'http://localhost:8080/api?payload=invalid-json'");
        // 4. 'name' key missing in JSON:
        System.out.println("curl 'http://localhost:8080/api?payload=%7B%22other_key%22%3A%22value%22%7D'");
        // 5. 'name' value is not a string:
        System.out.println("curl 'http://localhost:8080/api?payload=%7B%22name%22%3A123%7D'");
    }

    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            Map<String, String> params = queryToMap(exchange.getRequestURI().getQuery());
            String payload = params.get("payload");

            if (payload == null || payload.trim().isEmpty()) {
                sendResponse(exchange, 400, "'payload' parameter is missing");
                return;
            }

            try {
                JSONObject json = new JSONObject(payload);
                
                if (!json.has("name") || !(json.get("name") instanceof String)) {
                     sendResponse(exchange, 400, "'name' key is missing or not a string in JSON payload");
                     return;
                }

                String name = json.getString("name");
                sendResponse(exchange, 200, name);

            } catch (JSONException e) {
                sendResponse(exchange, 400, "Invalid JSON format in 'payload'");
            }
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
        
        private Map<String, String> queryToMap(String query) {
            if (query == null || query.isEmpty()) {
                return Collections.emptyMap();
            }
            Map<String, String> result = new HashMap<>();
            for (String param : query.split("&")) {
                String[] entry = param.split("=", 2);
                try {
                    String key = URLDecoder.decode(entry[0], StandardCharsets.UTF_8.name());
                    String value = entry.length > 1 ? URLDecoder.decode(entry[1], StandardCharsets.UTF_8.name()) : "";
                    result.put(key, value);
                } catch (UnsupportedEncodingException e) {
                    // Should not happen with UTF-8
                    // Log this exception in a real application
                }
            }
            return result;
        }
    }
}