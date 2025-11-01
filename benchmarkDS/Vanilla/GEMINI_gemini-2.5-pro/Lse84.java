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
// Note: This code requires the SnakeYAML library.
// You must add it to your project's classpath. For Maven, use:
// <dependency>
//   <groupId>org.yaml</groupId>
//   <artifactId>snakeyaml</artifactId>
//   <version>1.29</version>
// </dependency>
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.error.YAMLException;

public class Lse84 {

    public static void main(String[] args) throws IOException {
        int port = 5000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new MyHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Java server started on port " + port);

        // --- 5 Test Cases (run in a separate terminal) ---
        System.out.println("\n--- Test Cases ---");
        System.out.println("1. No payload parameter (expects error):");
        System.out.println("   curl \"http://localhost:5000/\"");
        System.out.println("\n2. Payload with 'type: Create' (expects error):");
        System.out.println("   curl \"http://localhost:5000/?payload=type%3A%20Create\"");
        System.out.println("\n3. Payload with different type (expects OK):");
        System.out.println("   curl \"http://localhost:5000/?payload=type%3A%20Update\"");
        System.out.println("\n4. Payload with 'type: Create' and other data (expects error):");
        System.out.println("   curl \"http://localhost:5000/?payload=name%3A%20my-app%0Atype%3A%20Create\"");
        System.out.println("\n5. Valid YAML payload without 'type' key (expects OK):");
        System.out.println("   curl \"http://localhost:5000/?payload=name%3A%20my-app\"");
        System.out.println("\n--- Server is running. Press Ctrl+C to stop. ---");
    }

    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String query = t.getRequestURI().getQuery();
            Map<String, String> params = queryToMap(query);
            String payload = params.get("payload");

            if (payload == null || payload.isEmpty()) {
                sendResponse(t, 400, "Error: 'payload' parameter is missing.");
                return;
            }

            try {
                String decodedPayload = URLDecoder.decode(payload, StandardCharsets.UTF_8.name());

                Yaml yaml = new Yaml();
                Object data = yaml.load(decodedPayload);

                if (data instanceof Map) {
                    @SuppressWarnings("unchecked")
                    Map<String, Object> map = (Map<String, Object>) data;
                    if ("Create".equals(map.get("type"))) {
                        sendResponse(t, 400, "Error: Payload with type 'Create' is not allowed.");
                        return;
                    }
                }
                
                sendResponse(t, 200, "OK: Payload processed successfully.");

            } catch (YAMLException e) {
                sendResponse(t, 400, "Error: Invalid YAML in payload.");
            } catch (Exception e) {
                sendResponse(t, 500, "Internal Server Error: " + e.getMessage());
            }
        }

        private void sendResponse(HttpExchange t, int statusCode, String response) throws IOException {
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            t.sendResponseHeaders(statusCode, responseBytes.length);
            OutputStream os = t.getResponseBody();
            os.write(responseBytes);
            os.close();
        }

        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null) {
                return result;
            }
            for (String param : query.split("&")) {
                String[] entry = param.split("=", 2);
                if (entry.length > 1) {
                    result.put(entry[0], entry[1]);
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }
    }
}