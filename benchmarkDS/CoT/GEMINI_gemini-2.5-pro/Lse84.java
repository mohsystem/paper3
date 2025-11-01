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
import java.util.HashMap;
import java.util.Map;

// Note: This code requires the SnakeYAML library.
// You can add it to your project, e.g., using Maven/Gradle.
// <dependency>
//     <groupId>org.yaml</groupId>
//     <artifactId>snakeyaml</artifactId>
//     <version>2.0</version>
// </dependency>

public class Lse84 {

    public static void main(String[] args) throws IOException {
        int port = 5000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new MyHandler());
        server.setExecutor(null); // creates a default executor
        System.out.println("Server started on port " + port);
        server.start();

        /*
         * Main method starts the server. To test, run the server and use a tool like curl:
         * 
         * Test Case 1: No payload parameter
         * curl "http://localhost:5000/"
         * Expected Output: Error: 'payload' parameter is missing.
         * 
         * Test Case 2: Payload with type: Create
         * curl "http://localhost:5000/?payload=type%3A%20Create%0Aname%3A%20my-resource"
         * Expected Output: Error: Payload with type 'Create' is not allowed.
         *
         * Test Case 3: Payload with a different type
         * curl "http://localhost:5000/?payload=type%3A%20Update%0Aname%3A%20my-resource"
         * Expected Output: Payload processed successfully.
         * 
         * Test Case 4: Malformed YAML payload
         * curl "http://localhost:5000/?payload=type%3A%20[invalid"
         * Expected Output: Error: Invalid YAML format in payload.
         *
         * Test Case 5: Valid YAML but no 'type' key
         * curl "http://localhost:5000/?payload=name%3A%20my-resource"
         * Expected Output: Payload processed successfully.
         */
    }

    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String query = t.getRequestURI().getQuery();
            Map<String, String> params = queryToMap(query);
            String response;
            int statusCode;

            if (params == null || !params.containsKey("payload")) {
                response = "Error: 'payload' parameter is missing.";
                statusCode = 400;
            } else {
                String payload = params.get("payload");
                String processingResult = processPayload(payload);
                if (processingResult.startsWith("Error:")) {
                    response = processingResult;
                    statusCode = 400;
                } else {
                    response = processingResult;
                    statusCode = 200;
                }
            }

            t.sendResponseHeaders(statusCode, response.getBytes().length);
            OutputStream os = t.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }

    public static String processPayload(String payload) {
        // Use SafeConstructor to prevent arbitrary code execution
        Yaml yaml = new Yaml(new SafeConstructor());
        try {
            Map<String, Object> data = yaml.load(payload);
            
            // data can be null for empty payload string
            if (data != null && "Create".equals(data.get("type"))) {
                return "Error: Payload with type 'Create' is not allowed.";
            } else {
                return "Payload processed successfully.";
            }
        } catch (YAMLException e) {
            return "Error: Invalid YAML format in payload.";
        }
    }

    public static Map<String, String> queryToMap(String query) {
        if (query == null) {
            return null;
        }
        Map<String, String> result = new HashMap<>();
        for (String param : query.split("&")) {
            String[] entry = param.split("=", 2);
            if (entry.length > 1) {
                try {
                    result.put(URLDecoder.decode(entry[0], "UTF-8"), URLDecoder.decode(entry[1], "UTF-8"));
                } catch (UnsupportedEncodingException e) {
                    // This should not happen with UTF-8
                    e.printStackTrace();
                }
            }
        }
        return result;
    }
}