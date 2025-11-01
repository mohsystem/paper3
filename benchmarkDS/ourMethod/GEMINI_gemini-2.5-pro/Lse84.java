/**
 * Required dependency: org.yaml.snakeyaml
 * Add to pom.xml:
 * <dependency>
 *     <groupId>org.yaml.snakeyaml</groupId>
 *     <artifactId>snakeyaml</artifactId>
 *     <version>2.0</version>
 * </dependency>
 * To compile and run from command line:
 * 1. Download snakeyaml-2.0.jar
 * 2. javac -cp "path/to/snakeyaml-2.0.jar" Lse84.java
 * 3. java -cp ".:path/to/snakeyaml-2.0.jar" Lse84
 */
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.error.YAMLException;

import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

public class Lse84 {

    private static final int PORT = 5000;
    private static final int MAX_PAYLOAD_SIZE = 4096;

    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/", new MyHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        System.out.println("Server started on port " + PORT);

        // Test cases
        System.out.println("\n--- Test Cases (run in another terminal) ---");
        System.out.println("1. No payload parameter:");
        System.out.println("   curl -v \"http://localhost:5000/\"");
        System.out.println("2. Payload with type 'Create' (should be rejected):");
        System.out.println("   curl -v \"http://localhost:5000/?payload=type%3A%20Create%0Aname%3A%20test\"");
        System.out.println("3. Payload with a different type (should be accepted):");
        System.out.println("   curl -v \"http://localhost:5000/?payload=type%3A%20Update%0Aname%3A%20test\"");
        System.out.println("4. Malformed YAML payload (should be rejected):");
        System.out.println("   curl -v \"http://localhost:5000/?payload=type%3A%20Create%3A%20Fail\"");
        System.out.println("5. Empty payload (should be rejected):");
        System.out.println("   curl -v \"http://localhost:5000/?payload=\"");
        System.out.println("\nPress Ctrl+C to stop the server.");
    }

    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;
            try {
                if ("GET".equals(exchange.getRequestMethod())) {
                    String query = exchange.getRequestURI().getQuery();
                    Map<String, String> params = queryToMap(query);
                    String payload = params.get("payload");

                    Map.Entry<Integer, String> result = processPayload(payload);
                    statusCode = result.getKey();
                    response = result.getValue();

                } else {
                    statusCode = 405; // Method Not Allowed
                    response = "Error: Method Not Allowed";
                }
            } catch (Exception e) {
                // Generic error to avoid leaking implementation details
                statusCode = 500;
                response = "Internal Server Error";
            }
            
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    public static Map.Entry<Integer, String> processPayload(String payload) {
        if (payload == null || payload.trim().isEmpty()) {
            return Map.entry(400, "Error: 'payload' parameter is missing or empty.");
        }

        if (payload.length() > MAX_PAYLOAD_SIZE) {
            return Map.entry(400, "Error: Payload exceeds maximum size limit.");
        }

        try {
            // Use SafeConstructor to prevent arbitrary code execution
            Yaml yaml = new Yaml(new SafeConstructor());
            Object data = yaml.load(payload);

            if (data instanceof Map) {
                @SuppressWarnings("unchecked")
                Map<String, Object> map = (Map<String, Object>) data;
                if ("Create".equals(map.get("type"))) {
                    return Map.entry(400, "Error: 'Create' type is not allowed.");
                }
            }
            return Map.entry(200, "Success: Payload processed.");

        } catch (YAMLException e) {
            return Map.entry(400, "Error: Invalid YAML format in payload.");
        } catch (Exception e) {
            return Map.entry(500, "Error: Could not process payload.");
        }
    }

    private static Map<String, String> queryToMap(String query) {
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
                // This should not happen with UTF-8
            }
        }
        return result;
    }
}