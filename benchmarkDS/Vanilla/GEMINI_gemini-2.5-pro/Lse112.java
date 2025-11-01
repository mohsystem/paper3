import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.stream.Collectors;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse112 {

    /**
     * Executes the system's ping command for a given URL.
     * @param url The URL or IP address to ping.
     * @return The output of the ping command as a String.
     */
    private static String executePing(String url) {
        // Basic validation to prevent command injection.
        // A simple regex to allow domain names and IP addresses.
        if (url == null || url.trim().isEmpty() || !url.matches("^[a-zA-Z0-9.\\-]+$")) {
            return "Error: Invalid or potentially malicious URL format provided.";
        }
        
        StringBuilder output = new StringBuilder();
        // Determine OS and build the appropriate command
        String os = System.getProperty("os.name").toLowerCase();
        String[] command;
        if (os.contains("win")) {
            command = new String[]{"ping", "-n", "4", url};
        } else {
            // Assume Linux, macOS, or other Unix-like OS
            command = new String[]{"ping", "-c", "4", url};
        }

        ProcessBuilder processBuilder = new ProcessBuilder(command);
        processBuilder.redirectErrorStream(true); // Merge stdout and stderr

        try {
            Process process = processBuilder.start();
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }
            int exitCode = process.waitFor();
            output.append(System.lineSeparator()).append("Exited with code: ").append(exitCode);
        } catch (IOException | InterruptedException e) {
            output.append("Error executing ping command: ").append(e.getMessage());
            Thread.currentThread().interrupt(); // Set the interrupt flag
        }
        return output.toString();
    }

    /**
     * HTTP handler for the /ping endpoint.
     */
    static class PingHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String urlToPing = "";
            URI requestUri = exchange.getRequestURI();
            String query = requestUri.getQuery();
            if (query != null && query.startsWith("url=")) {
                // Extract the URL from the query parameter "url=..."
                urlToPing = query.substring(4);
                // Decode the URL to handle any special characters
                urlToPing = URLDecoder.decode(urlToPing, StandardCharsets.UTF_8.name());
            }

            String response;
            if (urlToPing.isEmpty()) {
                response = "Please provide a URL to ping. Example: /ping?url=google.com";
            } else {
                response = executePing(urlToPing);
            }

            exchange.getResponseHeaders().set("Content-Type", "text/plain");
            exchange.sendResponseHeaders(200, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }
    
    /**
     * Runs 5 test cases directly from the command line.
     */
    private static void runTestCases() {
        System.out.println("--- Running 5 Command-Line Test Cases ---");
        String[] testUrls = {
            "google.com",               // Valid domain
            "127.0.0.1",                // Localhost IP
            "8.8.8.8",                  // Public IP
            "nonexistent-domain-1234.com", // Invalid domain
            "google.com; ls -la"        // Malicious input attempt
        };

        for (String url : testUrls) {
            System.out.println("\n--- Pinging: " + url + " ---");
            String result = executePing(url);
            System.out.println(result);
        }
    }

    public static void main(String[] args) throws IOException {
        // 1. Run 5 test cases and print results to console
        runTestCases();

        // 2. Start the web server
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/ping", new PingHandler());
        server.setExecutor(null); // creates a default executor
        
        System.out.println("\n--- Starting Web Server on port " + port + " ---");
        System.out.println("To test the server, open a browser or use curl with the following URLs:");
        System.out.println("  curl 'http://localhost:8080/ping?url=google.com'");
        System.out.println("  curl 'http://localhost:8080/ping?url=1.1.1.1'");
        System.out.println("Server started. Press Ctrl+C to stop.");
        server.start();
    }
}