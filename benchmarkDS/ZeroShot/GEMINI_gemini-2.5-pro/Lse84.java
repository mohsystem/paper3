import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.error.YAMLException;

import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Stream;
import java.util.stream.Collectors;

/*
NOTE: This code requires the SnakeYAML library.
Add the following dependency to your pom.xml if you are using Maven:
<dependency>
    <groupId>org.yaml</groupId>
    <artifactId>snakeyaml</artifactId>
    <version>2.0</version>
</dependency>
*/
public class Lse84 {

    public static void main(String[] args) throws IOException {
        int port = 5000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new MyHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Running 5 test cases using curl:");
        System.out.println("1. No 'payload' parameter: curl -i \"http://localhost:5000/test\"");
        System.out.println("2. 'payload' with 'type: Create': curl -i \"http://localhost:5000/?payload=type%3A%20Create\"");
        System.out.println("3. Valid 'payload': curl -i \"http://localhost:5000/?payload=type%3A%20Update%0Aname%3A%20test\"");
        System.out.println("4. Malformed YAML in 'payload': curl -i \"http://localhost:5000/?payload=type%3A%20%5Binvalid\"");
        System.out.println("5. Empty 'payload': curl -i \"http://localhost:5000/?payload=\"");
    }

    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String query = t.getRequestURI().getQuery();
            Map<String, String> params = queryToMap(query);

            String payload = params.get("payload");

            if (payload == null) {
                sendResponse(t, 400, "Error: 'payload' parameter is missing.");
                return;
            }
            if (payload.trim().isEmpty()) {
                sendResponse(t, 400, "Error: 'payload' parameter is empty.");
                return;
            }

            // Use SafeConstructor to prevent arbitrary code execution
            Yaml yaml = new Yaml(new SafeConstructor());
            try {
                Map<String, Object> data = yaml.load(payload);
                if (data == null) {
                    sendResponse(t, 400, "Error: Payload is not a valid YAML object.");
                    return;
                }

                Object type = data.get("type");
                if ("Create".equals(type)) {
                    sendResponse(t, 400, "Error: 'Create' type is not allowed.");
                } else {
                    sendResponse(t, 200, "Payload processed successfully.");
                }

            } catch (YAMLException e) {
                sendResponse(t, 400, "Error: Could not parse YAML payload. " + e.getMessage());
            } catch (ClassCastException e) {
                sendResponse(t, 400, "Error: Payload must be a YAML mapping (key-value pairs).");
            }
        }
    }

    private static void sendResponse(HttpExchange t, int statusCode, String response) throws IOException {
        t.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
        try (OutputStream os = t.getResponseBody()) {
            os.write(response.getBytes(StandardCharsets.UTF_8));
        }
    }

    private static Map<String, String> queryToMap(String query) {
        if (query == null || query.isEmpty()) {
            return new HashMap<>();
        }
        return Stream.of(query.split("&"))
                .map(s -> s.split("=", 2))
                .collect(Collectors.toMap(
                        a -> decode(a[0]),
                        a -> a.length > 1 ? decode(a[1]) : "",
                        (v1, v2) -> v1 // In case of duplicate keys, keep the first one
                ));
    }

    private static String decode(String value) {
        try {
            return URLDecoder.decode(value, StandardCharsets.UTF_8.toString());
        } catch (UnsupportedEncodingException ex) {
            // Should not happen, UTF-8 is always supported
            throw new RuntimeException(ex);
        }
    }
}