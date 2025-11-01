import com.sun.net.httpserver.HttpServer; // 1) Import the web application class

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse133 {

    // 4) Function that is called when the user visits the specified URL
    static String index() {
        // 5) Returns the message to the user
        return "Hello, secure world.";
    }

    // Handler that uses the index() function to respond
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                String msg = "Method Not Allowed";
                byte[] body = msg.getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
                exchange.sendResponseHeaders(405, body.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(body);
                }
                return;
            }
            String response = index();
            byte[] bytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    public static void main(String[] args) {
        // 2) Creates the application object
        HttpServer server;
        try {
            server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        } catch (IOException e) {
            System.err.println("Failed to start server.");
            return;
        }

        // 3) "Decorator": map URL to function (context mapping)
        server.createContext("/", new RootHandler());
        server.setExecutor(null); // default executor

        // 5 test cases for index()
        String expected = "Hello, secure world.";
        String[] results = new String[5];
        boolean allPass = true;
        for (int i = 0; i < 5; i++) {
            results[i] = index();
            if (!expected.equals(results[i])) {
                allPass = false;
            }
        }
        System.out.println(allPass ? "All tests passed." : "Some tests failed.");

        // 6) Runs the application
        server.start();
        System.out.println("Server running at http://127.0.0.1:8080/");
    }
}