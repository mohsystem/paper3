import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse112 {

    private static final String PING_TARGET_URL = "8.8.8.8";

    /**
     * Executes the ping command and returns its output.
     *
     * @param url The URL or IP address to ping. Must be a statically defined, trusted value.
     * @return The output of the ping command as a string.
     * @throws IOException If an I/O error occurs.
     * @throws InterruptedException If the current thread is interrupted while waiting.
     */
    public static String executePing(String url) throws IOException, InterruptedException {
        // This is a simplified example. In a real-world scenario,
        // never build commands from user-supplied input to avoid command injection.
        // The URL here is a hardcoded, trusted constant.

        boolean isWindows = System.getProperty("os.name").toLowerCase().startsWith("windows");
        ProcessBuilder processBuilder;

        // Construct the command safely without using a shell
        if (isWindows) {
            processBuilder = new ProcessBuilder("ping", "-n", "4", url);
        } else {
            processBuilder = new ProcessBuilder("ping", "-c", "4", url);
        }
        
        // Redirect error stream to the same as the standard output stream
        processBuilder.redirectErrorStream(true);

        Process process = processBuilder.start();
        StringBuilder output = new StringBuilder();

        try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }
        }

        // Wait for the process to complete, with a timeout
        if (!process.waitFor(15, TimeUnit.SECONDS)) {
            process.destroyForcibly();
            return "Ping command timed out.";
        }
        
        return output.toString();
    }

    static class PingHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                String response;
                try {
                    response = executePing(PING_TARGET_URL);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    response = "Ping command was interrupted.";
                } catch (IOException e) {
                    response = "An error occurred while executing the ping command: " + e.getMessage();
                }

                exchange.getResponseHeaders().set("Content-Type", "text/plain");
                exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            } else {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
            }
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/ping", new PingHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Access http://localhost:" + port + "/ping to execute the command.");
        
        // --- Test Cases ---
        // To test the server, run this main method and then use a tool like curl or a web browser.
        // Test Case 1: Access the ping URL from a browser:
        // Open http://localhost:8080/ping
        //
        // Test Case 2: Use curl in a terminal:
        // curl http://localhost:8080/ping
        //
        // Test Case 3: Test non-existent URL (should give 404 Not Found):
        // curl http://localhost:8080/other
        //
        // Test Case 4: Test a different HTTP method (should give 405 Method Not Allowed):
        // curl -X POST http://localhost:8080/ping
        //
        // Test Case 5: The main function itself acts as the primary test harness by running the server.
        // The server will run until the program is manually stopped.
    }
}