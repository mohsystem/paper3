
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse112 {
    // Whitelist pattern for valid hostnames/IPs - only alphanumeric, dots, hyphens
    // Prevents command injection by strictly validating input format
    private static final Pattern VALID_HOST_PATTERN = Pattern.compile("^[a-zA-Z0-9.-]+$");
    private static final int MAX_HOST_LENGTH = 253; // RFC 1035 max domain length

    // Ping handler bound to /ping endpoint
    static class PingHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;

            try {
                // Extract URL parameter from query string
                String query = exchange.getRequestURI().getQuery();
                String url = extractUrlParameter(query);

                if (url == null || url.isEmpty()) {
                    response = "Error: Missing 'url' parameter";
                    statusCode = 400;
                } else if (!isValidHost(url)) {
                    // Input validation: reject invalid hostnames to prevent command injection
                    response = "Error: Invalid URL format";
                    statusCode = 400;
                } else {
                    // Sanitized execution: use ProcessBuilder with separate arguments
                    // This prevents shell injection as no shell is involved
                    response = executePing(url);
                    statusCode = 200;
                }
            } catch (Exception e) {
                // Fail closed: return generic error without exposing internal details
                response = "Error: Request processing failed";
                statusCode = 500;
            }

            // Send response back to browser
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }

        private String extractUrlParameter(String query) {
            if (query == null) return null;
            String[] pairs = query.split("&");
            for (String pair : pairs) {
                String[] keyValue = pair.split("=", 2);
                if (keyValue.length == 2 && "url".equals(keyValue[0])) {
                    return keyValue[1];
                }
            }
            return null;
        }

        private boolean isValidHost(String host) {
            // Validate length to prevent buffer issues
            if (host.length() > MAX_HOST_LENGTH) return false;
            // Validate format: only safe characters allowed
            // Rejects shell metacharacters like ;|&$()` etc.
            return VALID_HOST_PATTERN.matcher(host).matches();
        }

        private String executePing(String host) {
            try {
                // Use ProcessBuilder with argument array - prevents shell injection
                // ping command limited to 4 packets for safety
                ProcessBuilder pb = new ProcessBuilder("ping", "-c", "4", host);
                pb.redirectErrorStream(true);
                Process process = pb.start();

                // Read output with bounded buffer to prevent memory exhaustion
                StringBuilder output = new StringBuilder();
                byte[] buffer = new byte[4096];
                int bytesRead;
                int totalBytes = 0;
                int maxOutput = 65536; // 64KB limit

                while ((bytesRead = process.getInputStream().read(buffer)) != -1) {
                    totalBytes += bytesRead;
                    if (totalBytes > maxOutput) {
                        process.destroy();
                        return "Error: Output too large";
                    }
                    output.append(new String(buffer, 0, bytesRead, StandardCharsets.UTF_8));
                }

                process.waitFor();
                return "Ping result for " + host + ":\\n" + output.toString();
            } catch (Exception e) {
                // Generic error message - no internal details leaked
                return "Error: Ping execution failed";
            }
        }
    }

    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/ping", new PingHandler());
        server.setExecutor(null);
        server.start();

        System.out.println("Server started on port 8080");
        System.out.println("Test cases:");
        System.out.println("1. http://localhost:8080/ping?url=localhost");
        System.out.println("2. http://localhost:8080/ping?url=127.0.0.1");
        System.out.println("3. http://localhost:8080/ping?url=google.com");
        System.out.println("4. http://localhost:8080/ping?url=");
        System.out.println("5. http://localhost:8080/ping?url=invalid;command");
    }
}
