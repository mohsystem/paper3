import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.net.URLDecoder;

public class Lse83 {

    /**
     * Parses a URL-encoded string payload and extracts the value of the 'name' field.
     * @param payload The URL-encoded string (e.g., "id=123&name=JohnDoe").
     * @return The value of the 'name' field, or a "Name not found" message.
     */
    public static String processPayload(String payload) {
        if (payload == null || payload.trim().isEmpty()) {
            return "Payload is empty or null.";
        }
        
        Map<String, String> params = new HashMap<>();
        try {
            String[] pairs = payload.split("&");
            for (String pair : pairs) {
                int idx = pair.indexOf("=");
                if (idx > 0 && idx < pair.length() - 1) {
                    params.put(
                        URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8.name()),
                        URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8.name())
                    );
                }
            }
        } catch (Exception e) {
            // Handles potential UnsupportedEncodingException or other parsing errors
            return "Error parsing payload: " + e.getMessage();
        }

        return params.getOrDefault("name", "Field 'name' not found in payload.");
    }

    public static void main(String[] args) throws IOException {
        // --- 5 Test Cases for the core logic ---
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: 'name' field exists
        String payload1 = "id=1&name=Alice&role=user";
        System.out.println("Test 1 Payload: " + payload1);
        System.out.println("Test 1 Result: " + processPayload(payload1)); // Expected: Alice

        // Test Case 2: 'name' field is missing
        String payload2 = "id=2&role=admin";
        System.out.println("Test 2 Payload: " + payload2);
        System.out.println("Test 2 Result: " + processPayload(payload2)); // Expected: Field 'name' not found...

        // Test Case 3: Empty payload
        String payload3 = "";
        System.out.println("Test 3 Payload: (empty)");
        System.out.println("Test 3 Result: " + processPayload(payload3)); // Expected: Payload is empty...

        // Test Case 4: Null payload
        String payload4 = null;
        System.out.println("Test 4 Payload: (null)");
        System.out.println("Test 4 Result: " + processPayload(payload4)); // Expected: Payload is empty...
        
        // Test Case 5: 'name' is the only field
        String payload5 = "name=Bob";
        System.out.println("Test 5 Payload: " + payload5);
        System.out.println("Test 5 Result: " + processPayload(payload5)); // Expected: Bob
        System.out.println("--------------------------\n");

        // --- Start the web server ---
        int port = 8000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api", new MyHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Send a POST request to http://localhost:8000/api with a urlencoded body.");
        System.out.println("Example: curl -X POST -H \"Content-Type: application/x-www-form-urlencoded\" -d \"name=John Doe&age=30\" http://localhost:8000/api");
    }

    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String response = "This endpoint only accepts POST requests.";
            int statusCode = 405; // Method Not Allowed

            if ("POST".equalsIgnoreCase(t.getRequestMethod())) {
                StringBuilder sb = new StringBuilder();
                try (InputStreamReader isr = new InputStreamReader(t.getRequestBody(), StandardCharsets.UTF_8);
                     BufferedReader br = new BufferedReader(isr)) {
                    int b;
                    while ((b = br.read()) != -1) {
                        sb.append((char) b);
                    }
                }
                
                String payload = sb.toString();
                response = processPayload(payload);
                statusCode = 200; // OK
            }

            t.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = t.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }
}