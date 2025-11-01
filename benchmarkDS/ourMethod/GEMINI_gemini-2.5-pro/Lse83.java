import java.io.IOException;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.nio.charset.StandardCharsets;

public class Lse83 {

    /**
     * Processes a JSON-like string payload to extract the value of the "name" field.
     * This is a very basic parser for demonstration and avoids complex dependencies.
     * It is not a full-featured or robust JSON parser.
     *
     * @param payload The string payload to process.
     * @return The value of the "name" field, or null if not found or if the format is invalid.
     */
    public static String processPayload(String payload) {
        if (payload == null || payload.trim().isEmpty()) {
            return null;
        }

        // A simple, non-regex search for the "name" key.
        String key = "\"name\"";
        int keyIndex = payload.indexOf(key);
        if (keyIndex == -1) {
            return null;
        }

        // Look for the colon after the key
        int colonIndex = payload.indexOf(':', keyIndex + key.length());
        if (colonIndex == -1) {
            return null;
        }

        // Look for the opening quote of the value
        int valueStartIndex = payload.indexOf('"', colonIndex + 1);
        if (valueStartIndex == -1) {
            return null;
        }

        // Look for the closing quote of the value
        int valueEndIndex = payload.indexOf('"', valueStartIndex + 1);
        if (valueEndIndex == -1) {
            return null;
        }

        // Extract the value
        return payload.substring(valueStartIndex + 1, valueEndIndex);
    }

    public static void main(String[] args) throws IOException {
        // --- Test Cases ---
        System.out.println("--- Running Test Cases ---");
        String[] testPayloads = {
            "{\"name\":\"Alice\"}",
            "{\"age\":30, \"name\":\"Bob\"}",
            "   { \"name\" : \"Charlie\" }   ",
            "{\"city\":\"New York\"}",
            "invalid json",
        };
        String[] expectedResults = {"Alice", "Bob", "Charlie", null, null};

        for (int i = 0; i < testPayloads.length; i++) {
            String result = processPayload(testPayloads[i]);
            System.out.printf("Test Case %d: Payload='%s'%n", i + 1, testPayloads[i]);
            System.out.printf("  -> Expected: %s, Got: %s. Correct: %b%n",
                              expectedResults[i], result,
                              (expectedResults[i] == null ? result == null : expectedResults[i].equals(result)));
        }
        System.out.println("--- Test Cases Finished ---\n");


        // --- Web Server Setup ---
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/api/name", new NameHandler());
        server.setExecutor(null); // creates a default executor
        System.out.println("Server started on port 8080. Send a POST request to /api/name");
        System.out.println("Example: curl -X POST -H \"Content-Type: application/json\" -d '{\"name\":\"YourName\"}' http://localhost:8080/api/name");
        server.start();
    }

    static class NameHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "Invalid request";
            int statusCode = 400;

            try {
                if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    // Set a max payload size to prevent DoS attacks
                    final int MAX_PAYLOAD_SIZE = 1024;
                    StringBuilder payloadBuilder = new StringBuilder();
                    try (BufferedReader reader = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))) {
                        char[] buffer = new char[256];
                        int bytesRead;
                        int totalBytesRead = 0;
                        while ((bytesRead = reader.read(buffer)) != -1) {
                            totalBytesRead += bytesRead;
                            if (totalBytesRead > MAX_PAYLOAD_SIZE) {
                                statusCode = 413; // Payload Too Large
                                response = "Payload exceeds maximum size of " + MAX_PAYLOAD_SIZE + " bytes.";
                                // Drain the rest of the input before responding
                                while(reader.read(buffer) != -1);
                                break;
                            }
                            payloadBuilder.append(buffer, 0, bytesRead);
                        }
                    }

                    if (statusCode != 413) {
                        String name = processPayload(payloadBuilder.toString());
                        if (name != null) {
                            response = name;
                            statusCode = 200;
                        } else {
                            response = "Field 'name' not found or payload is malformed.";
                            statusCode = 400;
                        }
                    }
                } else {
                    response = "Method not allowed. Please use POST.";
                    statusCode = 405;
                }
            } catch (Exception e) {
                // Do not leak internal details in the response
                response = "An internal server error occurred.";
                statusCode = 500;
                // Log the actual error for debugging
                System.err.println("Error processing request: " + e.getMessage());
            }

            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }
}