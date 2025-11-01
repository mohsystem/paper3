import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.Scanner;
import java.util.concurrent.Executors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse83 {

    /**
     * Extracts the value of the "name" field from a JSON-like string payload.
     * This is a simple parser for demonstration; a real application should use a robust JSON library.
     * @param payload The request body as a string.
     * @return The value of the "name" field, or null if not found or malformed.
     */
    private static String extractNameFromPayload(String payload) {
        if (payload == null || payload.isEmpty()) {
            return null;
        }

        // Simple and insecure parsing for demonstration.
        // A secure implementation should use a proper JSON parser to avoid parsing vulnerabilities
        // and handle edge cases like escaped quotes.
        String searchKey = "\"name\":\"";
        int startIndex = payload.indexOf(searchKey);
        if (startIndex == -1) {
            return null;
        }
        startIndex += searchKey.length();

        int endIndex = payload.indexOf("\"", startIndex);
        if (endIndex == -1) {
            return null; // Malformed
        }
        
        // Basic validation: ensure the extracted value is not excessively long to prevent DoS.
        if (endIndex - startIndex > 1024) {
             return "[Error: Name is too long]";
        }

        return payload.substring(startIndex, endIndex);
    }

    static class NameHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String responseText = "Field 'name' not found or invalid request.";
            int statusCode = 400; // Bad Request

            // Security: Only allow POST requests to prevent CSRF on GET and adhere to HTTP semantics.
            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                try (InputStream is = exchange.getRequestBody();
                     Scanner s = new Scanner(is, StandardCharsets.UTF_8.name()).useDelimiter("\\A")) {
                    String payload = s.hasNext() ? s.next() : "";
                    String name = extractNameFromPayload(payload);

                    if (name != null) {
                        // Security: Return as plain text to mitigate XSS risks.
                        // If returning as HTML, the 'name' must be properly escaped/encoded.
                        responseText = name;
                        statusCode = 200; // OK
                    }
                } catch (Exception e) {
                    responseText = "Error processing request.";
                    statusCode = 500; // Internal Server Error
                }
            } else {
                responseText = "Method not allowed. Please use POST.";
                statusCode = 405; // Method Not Allowed
            }

            // Set content type to plain text for security
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            byte[] responseBytes = responseText.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/checkName", new NameHandler());
        // Use a fixed thread pool to prevent resource exhaustion
        server.setExecutor(Executors.newFixedThreadPool(10)); 
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Use a tool like curl to test the 5 cases below:");
        System.out.println("---");
        
        System.out.println("Test Case 1: Valid name");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"user\": \"guest\", \"name\": \"Alice\"}' http://localhost:8080/checkName");
        
        System.out.println("\nTest Case 2: No 'name' field");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"user\": \"guest\", \"id\": 123}' http://localhost:8080/checkName");

        System.out.println("\nTest Case 3: Empty payload");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '' http://localhost:8080/checkName");
        
        System.out.println("\nTest Case 4: Name with special characters (should be returned as is for text/plain)");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"Bob <script>alert(1)</script>\"}' http://localhost:8080/checkName");

        System.out.println("\nTest Case 5: Use GET method (should be rejected)");
        System.out.println("curl -X GET http://localhost:8080/checkName");
    }
}