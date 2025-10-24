import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class Task130 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/data", new CorsHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Java server started on port " + port);
        System.out.println("API endpoint available at http://localhost:8080/api/data");

        // --- Test Cases ---
        // To test the CORS implementation, run this server and then use a tool like curl
        // or a web page from a different origin to make requests.
        System.out.println("\n--- How to Test (run from another terminal) ---");
        System.out.println("1. Test Preflight (OPTIONS request):");
        System.out.println("   curl -v -X OPTIONS http://localhost:8080/api/data -H \"Origin: http://example.com\" -H \"Access-Control-Request-Method: GET\"");
        System.out.println("\n2. Test Actual Request from an allowed origin:");
        System.out.println("   curl -v -H \"Origin: http://example.com\" http://localhost:8080/api/data");
        System.out.println("\n3. Test with a different allowed origin (since we use '*'):");
        System.out.println("   curl -v -H \"Origin: http://another-site.com\" http://localhost:8080/api/data");
        System.out.println("\n4. Test a non-CORS request (no Origin header):");
        System.out.println("   curl -v http://localhost:8080/api/data");
        System.out.println("\n5. Test with a browser. Create an HTML file with the following JS and open it:");
        System.out.println("   fetch('http://localhost:8080/api/data').then(r => r.json()).then(console.log).catch(console.error);");

    }

    static class CorsHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Set CORS headers
            // Allow all origins. For production, you should restrict this to specific domains.
            // e.g., "http://your-frontend-domain.com"
            exchange.getResponseHeaders().add("Access-Control-Allow-Origin", "*");
            exchange.getResponseHeaders().add("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            exchange.getResponseHeaders().add("Access-Control-Allow-Headers", "Content-Type,Authorization");
            
            // Handle preflight (OPTIONS) requests
            if (exchange.getRequestMethod().equalsIgnoreCase("OPTIONS")) {
                exchange.sendResponseHeaders(204, -1); // No Content
                return;
            }

            // Handle actual API (GET) request
            if (exchange.getRequestMethod().equalsIgnoreCase("GET")) {
                String response = "{\"message\": \"Hello from Java API with CORS!\"}";
                exchange.getResponseHeaders().set("Content-Type", "application/json");
                exchange.sendResponseHeaders(200, response.getBytes().length);
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            } else {
                 // Method not allowed
                exchange.sendResponseHeaders(405, -1);
            }
        }
    }
}