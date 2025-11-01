import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class Lse133 {

    // Function that accepts input as parameter and returns output
    public static String greet(String name) {
        return "Hello, " + name + "!";
    }

    // Start a simple HTTP server; returns the port it started on or -1 on failure
    public static int startServer(int port) {
        try {
            HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
            server.createContext("/", new HttpHandler() {
                @Override
                public void handle(HttpExchange exchange) throws IOException {
                    String response = greet("Java User");
                    exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=UTF-8");
                    exchange.sendResponseHeaders(200, response.getBytes("UTF-8").length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(response.getBytes("UTF-8"));
                    }
                }
            });
            server.setExecutor(null);
            server.start();
            return port;
        } catch (IOException e) {
            e.printStackTrace();
            return -1;
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        // 5 test cases for greet function
        System.out.println(greet("Alice"));
        System.out.println(greet("Bob"));
        System.out.println(greet("Charlie"));
        System.out.println(greet("Dana"));
        System.out.println(greet("Eve"));

        // Start server on port 8080
        int port = 8080;
        int startedPort = startServer(port);
        if (startedPort != -1) {
            System.out.println("Java HTTP server running on http://localhost:" + startedPort + "/");
        } else {
            System.out.println("Failed to start Java HTTP server.");
        }
    }
}