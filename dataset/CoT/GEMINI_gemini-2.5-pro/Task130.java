import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executors;

public class Task130 {

    // Whitelist of allowed origins for CORS
    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
        "http://localhost:3000",
        "https://safe.example.com"
    ));

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/data", new CorsApiHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        System.out.println("Java server started on port " + port);
        System.out.println("Running tests...");
        runTests();
        System.out.println("Tests complete. Server is still running. Press Ctrl+C to stop.");
    }

    static class CorsApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String requestOrigin = exchange.getRequestHeaders().getFirst("Origin");

            // Secure CORS: Check if the origin is in the whitelist
            if (requestOrigin != null && ALLOWED_ORIGINS.contains(requestOrigin)) {
                exchange.getResponseHeaders().set("Access-Control-Allow-Origin", requestOrigin);
                exchange.getResponseHeaders().set("Access-Control-Allow-Methods", "GET, OPTIONS");
                exchange.getResponseHeaders().set("Access-Control-Allow-Headers", "Content-Type, X-Requested-With");
            }

            String requestMethod = exchange.getRequestMethod();
            if (requestMethod.equalsIgnoreCase("OPTIONS")) {
                // Handle preflight request
                exchange.sendResponseHeaders(204, -1); // No Content
            } else if (requestMethod.equalsIgnoreCase("GET")) {
                // Handle actual API request
                String response = "{\"message\": \"Hello from Java CORS API!\"}";
                exchange.getResponseHeaders().set("Content-Type", "application/json");
                exchange.sendResponseHeaders(200, response.getBytes().length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes());
                }
            } else {
                // Method not allowed
                exchange.sendResponseHeaders(405, -1);
            }
            exchange.close();
        }
    }

    // A simple client to demonstrate the 5 test cases
    public static void runTests() {
        HttpClient client = HttpClient.newHttpClient();
        String serverUrl = "http://localhost:8080/api/data";

        System.out.println("\n--- Test Case 1: Preflight OPTIONS from allowed origin ---");
        testRequest(client, "OPTIONS", serverUrl, "https://safe.example.com");

        System.out.println("\n--- Test Case 2: GET from allowed origin ---");
        testRequest(client, "GET", serverUrl, "https://safe.example.com");

        System.out.println("\n--- Test Case 3: GET from disallowed origin ---");
        testRequest(client, "GET", serverUrl, "http://evil.com");

        System.out.println("\n--- Test Case 4: GET with no Origin header ---");
        testRequest(client, "GET", serverUrl, null);

        System.out.println("\n--- Test Case 5: GET request to a non-existent path ---");
        testRequest(client, "GET", "http://localhost:8080/other/path", "https://safe.example.com");
    }

    private static void testRequest(HttpClient client, String method, String url, String origin) {
        try {
            HttpRequest.Builder requestBuilder = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .method(method, HttpRequest.BodyPublishers.noBody());
            
            if (origin != null) {
                requestBuilder.header("Origin", origin);
            }
            // For OPTIONS, also add the headers the actual request would have
            if ("OPTIONS".equals(method)) {
                 requestBuilder.header("Access-Control-Request-Method", "GET");
                 requestBuilder.header("Access-Control-Request-Headers", "Content-Type");
            }

            HttpRequest request = requestBuilder.build();
            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

            System.out.println("Request: " + method + " " + url + " | Origin: " + (origin != null ? origin : "none"));
            System.out.println("Response Status: " + response.statusCode());
            System.out.println("Access-Control-Allow-Origin Header: " + response.headers().firstValue("Access-Control-Allow-Origin").orElse("Not present"));
            if (!response.body().isEmpty()) {
                System.out.println("Response Body: " + response.body());
            }
        } catch (Exception e) {
            System.err.println("Error during test request: " + e.getMessage());
        }
    }
}