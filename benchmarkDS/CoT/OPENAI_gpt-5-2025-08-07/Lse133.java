import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpExchange;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

public class Lse133 {

    // Function that accepts an input (URL path) and returns a message
    public static String messageForPath(String path) {
        if ("/".equals(path)) {
            return "Hello from Java WebApp";
        }
        return "404 Not Found";
    }

    // Creates the application object (web server)
    public static HttpServer createServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        // Map URL "/" to associated function (acts like a route mapping)
        server.createContext("/", (HttpExchange exchange) -> {
            String response = messageForPath("/");
            byte[] bytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        });
        server.setExecutor(null);
        return server;
    }

    // Simple test routine with 5 test cases
    public static void runTests() {
        String[] testPaths = {"/", "/hello", "/", "/unknown", "/"};
        for (String p : testPaths) {
            String result = messageForPath(p);
            System.out.println("Test path: " + p + " -> " + result);
        }
    }

    // Runs the application
    public static void main(String[] args) throws Exception {
        HttpServer server = createServer(8080);
        server.start();
        runTests();
        System.out.println("Java HttpServer running at http://localhost:8080/");
        // Keep the main thread alive briefly
        Thread.sleep(2000);
    }
}