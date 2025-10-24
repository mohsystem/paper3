import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.Headers;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.concurrent.Executors;

public class Task130 {

    public static void main(String[] args) {
        int port = 8080;
        try {
            // Create a server on the specified port
            HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
            
            // Create a context for the API endpoint
            server.createContext("/api/data", new CorsApiHandler());
            
            // Use a fixed thread pool to handle requests
            server.setExecutor(Executors.newFixedThreadPool(10));
            
            System.out.println("Java server started on port " + port + "...");
            System.out.println("API endpoint available at http://localhost:" + port + "/api/data");
            
            // Display test cases
            runTests(port);

            // Start the server
            server.start();

        } catch (IOException e) {
            System.err.println("Could not start server: " + e.getMessage());
            e.printStackTrace();
        }
    }

    static class CorsApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Set CORS headers for all responses.
            // For enhanced security, restrict the origin to your frontend's domain instead of "*".
            // e.g., headers.add("Access-Control-Allow-Origin", "https://your-frontend-domain.com");
            Headers headers = exchange.getResponseHeaders();
            headers.add("Access-Control-Allow-Origin", "*");
            headers.add("Access-Control-Allow-Methods", "GET, OPTIONS");
            headers.add("Access-Control-Allow-Headers", "Content-Type,Authorization");
            headers.add("Access-Control-Max-Age", "86400"); // Cache preflight for 1 day

            try {
                if ("OPTIONS".equalsIgnoreCase(exchange.getRequestMethod())) {
                    handleOptionsRequest(exchange);
                } else if ("GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    handleGetRequest(exchange);
                } else {
                    handleMethodNotAllowed(exchange);
                }
            } finally {
                exchange.close();
            }
        }

        private void handleOptionsRequest(HttpExchange exchange) throws IOException {
            // This is a preflight request. The browser is asking for permission.
            // Respond with 204 No Content and the CORS headers are already added.
            exchange.sendResponseHeaders(204, -1);
        }

        private void handleGetRequest(HttpExchange exchange) throws IOException {
            String responseBody = "{\"message\": \"Hello from a CORS-enabled Java API!\"}";
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(200, responseBody.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBody.getBytes());
            }
        }

        private void handleMethodNotAllowed(HttpExchange exchange) throws IOException {
            String responseBody = "{\"error\": \"405 Method Not Allowed\"}";
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(405, responseBody.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBody.getBytes());
            }
        }
    }

    public static void runTests(int port) {
        System.out.println("\n--- How to Test ---");
        System.out.println("Run the following cURL commands in a separate terminal:");
        System.out.println("1. Test Preflight (OPTIONS) request for CORS:");
        System.out.println("   curl -i -X OPTIONS http://localhost:" + port + "/api/data -H \"Origin: http://example.com\" -H \"Access-Control-Request-Method: GET\"");
        System.out.println("\n2. Test actual (GET) request from a simulated foreign origin:");
        System.out.println("   curl -i -X GET http://localhost:" + port + "/api/data -H \"Origin: http://example.com\"");
        System.out.println("\n3. Test GET request without an Origin header (like a server-to-server call):");
        System.out.println("   curl -i http://localhost:" + port + "/api/data");
        System.out.println("\n4. Test a non-existent API endpoint:");
        System.out.println("   curl -i http://localhost:" + port + "/api/other");
        System.out.println("\n5. Test a disallowed method (e.g., POST) on the API endpoint:");
        System.out.println("   curl -i -X POST http://localhost:" + port + "/api/data -H \"Origin: http://example.com\"");
        System.out.println("--------------------\n");
    }
}