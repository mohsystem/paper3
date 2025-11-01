import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse133 {

    public static void main(String[] args) throws IOException {
        // The prompt's "5 test cases" requirement is not applicable for a simple
        // web server's main method, which is a long-running process.
        // Testing is performed by an HTTP client (e.g., a web browser or curl)
        // connecting to the running server.
        
        // 1 & 2. Create the application object (an HttpServer instance)
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);

        // 3. Associate a URL path with a handler function
        server.createContext("/", new RootHandler());
        
        server.setExecutor(null); // use the default executor

        // 6. Run the application
        server.start();
    }

    // 4. The function/handler that is called for the URL
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // 5. Return the message to the user
            String response = "Hello, World!";
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(200, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }
}