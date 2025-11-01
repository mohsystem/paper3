import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.stream.Collectors;

public class Lse83 {

    /**
     * Core logic function to find the 'name' field in a JSON-like string payload.
     * This is a very basic parser and not robust for complex or malformed JSON.
     * @param payload The string payload from the request body.
     * @return The value of the 'name' field, or an error message.
     */
    public static String checkNameInPayload(String payload) {
        // A simple, non-robust search for the "name":"value" pattern.
        String keyToFind = "\"name\"";
        int keyIndex = payload.indexOf(keyToFind);

        if (keyIndex == -1) {
            return "Error: 'name' field not found.";
        }

        int colonIndex = payload.indexOf(':', keyIndex + keyToFind.length());
        if (colonIndex == -1) {
            return "Error: Malformed payload, missing colon after name key.";
        }

        int valueStartIndex = payload.indexOf('"', colonIndex);
        if (valueStartIndex == -1) {
            return "Error: Malformed payload, expecting a string value for name.";
        }

        int valueEndIndex = payload.indexOf('"', valueStartIndex + 1);
        if (valueEndIndex == -1) {
            return "Error: Malformed payload, unclosed string value for name.";
        }

        return payload.substring(valueStartIndex + 1, valueEndIndex);
    }

    static class NameCheckHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "";
            int statusCode = 200;

            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                try (InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                     BufferedReader br = new BufferedReader(isr)) {
                    
                    String payload = br.lines().collect(Collectors.joining("\n"));
                    
                    if (payload == null || payload.trim().isEmpty()) {
                        statusCode = 400; // Bad Request
                        response = "Error: Request body is empty.";
                    } else {
                        String nameValue = checkNameInPayload(payload);
                        if (nameValue.startsWith("Error:")) {
                           statusCode = 400; // Bad Request
                           response = nameValue;
                        } else {
                           response = nameValue;
                        }
                    }
                } catch (Exception e) {
                    statusCode = 500; // Internal Server Error
                    response = "Error: Could not process request. " + e.getMessage();
                }
            } else {
                statusCode = 405; // Method Not Allowed
                response = "Error: Only POST method is allowed.";
            }

            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // --- Test cases for the core logic function ---
        System.out.println("--- Testing core logic function ---");
        System.out.println("Test Case 1: {\"name\":\"Alice\"} -> " + checkNameInPayload("{\"name\":\"Alice\"}"));
        System.out.println("Test Case 2: {\"age\":30, \"name\":\"Bob\"} -> " + checkNameInPayload("{\"age\":30, \"name\":\"Bob\"}"));
        System.out.println("Test Case 3: {\"city\":\"New York\"} -> " + checkNameInPayload("{\"city\":\"New York\"}"));
        System.out.println("Test Case 4: Malformed payload {\"name\":Charlie} -> " + checkNameInPayload("{\"name\":Charlie}"));
        System.out.println("Test Case 5: Empty payload -> " + checkNameInPayload(""));
        System.out.println("----------------------------------\n");
        
        // --- Setting up and starting the web server ---
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/check_name", new NameCheckHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Send a POST request to http://localhost:8080/check_name");
        System.out.println("\n--- How to Test with curl ---");
        System.out.println("1. Success: curl -X POST -H \"Content-Type: application/json\" -d '{\"name\":\"Alice\"}' http://localhost:8080/check_name");
        System.out.println("2. Success with other fields: curl -X POST -H \"Content-Type: application/json\" -d '{\"age\": 25, \"name\":\"Bob\"}' http://localhost:8080/check_name");
        System.out.println("3. Field not found: curl -X POST -H \"Content-Type: application/json\" -d '{\"age\": 30}' http://localhost:8080/check_name");
        System.out.println("4. Empty payload: curl -X POST -d '' http://localhost:8080/check_name");
        System.out.println("5. Wrong method: curl -X GET http://localhost:8080/check_name");
    }
}