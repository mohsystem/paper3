import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.concurrent.Executors;
import java.nio.charset.StandardCharsets;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse112 {

    // The URL/IP to ping is a hardcoded, trusted constant to prevent command injection.
    private static final String URL_TO_PING = "8.8.8.8";

    public static void main(String[] args) throws IOException {
        /*
         * Test Cases:
         * 1. Start the server.
         * 2. Open a terminal or command prompt.
         * 3. Run the following command: curl http://localhost:8080/ping
         *    - This should return the output of the ping command.
         * 4. Run the following command: curl http://localhost:8080/other
         *    - This should return a 404 Not Found error.
         * 5. The code internally pings a fixed address (8.8.8.8) for security.
         *    The following conceptual tests are handled by the secure design:
         *    - Test with malicious input (e.g., curl "http://localhost:8080/ping?url=;ls"):
         *      This is not possible as the endpoint does not accept external input for the command.
         */
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/ping", new PingHandler());
        server.setExecutor(Executors.newCachedThreadPool()); 
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Access http://localhost:8080/ping to execute the ping command.");
    }

    static class PingHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                try {
                    String response = executePing(URL_TO_PING);
                    exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=" + StandardCharsets.UTF_8.name());
                    exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(response.getBytes(StandardCharsets.UTF_8));
                    }
                } catch (Exception e) {
                    String errorResponse = "Error executing ping command: " + e.getMessage();
                    exchange.sendResponseHeaders(500, errorResponse.getBytes(StandardCharsets.UTF_8).length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(errorResponse.getBytes(StandardCharsets.UTF_8));
                    }
                }
            } else {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
            }
            exchange.close();
        }
    }

    /**
     * Executes the ping command securely.
     * @param url The URL or IP address to ping.
     * @return The output from the ping command.
     * @throws IOException if an I/O error occurs.
     * @throws InterruptedException if the current thread is interrupted while waiting.
     */
    public static String executePing(String url) throws IOException, InterruptedException {
        // Using ProcessBuilder is the standard secure way to execute system commands in Java.
        // It separates the command and its arguments, preventing shell injection.
        
        // Determine the correct ping command based on the operating system.
        String os = System.getProperty("os.name").toLowerCase();
        ProcessBuilder processBuilder;
        if (os.contains("win")) {
            // For Windows: ping -n 4 <url>
            processBuilder = new ProcessBuilder("ping", "-n", "4", url);
        } else {
            // For Linux/macOS: ping -c 4 <url>
            processBuilder = new ProcessBuilder("ping", "-c", "4", url);
        }

        processBuilder.redirectErrorStream(true); // Combine stdout and stderr
        Process process = processBuilder.start();

        StringBuilder output = new StringBuilder();
        // Use try-with-resources to ensure the reader is closed
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }
        }

        int exitCode = process.waitFor();
        output.append("\nExited with code: ").append(exitCode);

        return output.toString();
    }
}