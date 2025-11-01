import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Lse32 {

    /**
     * Handles the redirect logic for a given URL string.
     * It writes an HTTP 302 redirect response if the URL is valid,
     * otherwise it writes a 400 Bad Request error response.
     *
     * @param exchange The HttpExchange object for the current request.
     * @param urlString The URL to validate and redirect to.
     * @throws IOException If an I/O error occurs.
     */
    public static void redirect_invalid_url(HttpExchange exchange, String urlString) throws IOException {
        try {
            // Rule #1: Validate input URL
            URI uri = new URI(urlString);
            String scheme = uri.getScheme();
            String host = uri.getHost();

            // Security: Check for a valid scheme (http or https) and a whitelisted host.
            // This prevents open redirect vulnerabilities (CWE-601).
            if (scheme != null && (scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https")) &&
                host != null && host.equalsIgnoreCase("example.com")) {
                
                // Valid URL, perform redirect
                exchange.getResponseHeaders().set("Location", uri.toString());
                exchange.sendResponseHeaders(302, -1); // 302 Found for temporary redirect
            } else {
                // Invalid host or scheme
                sendErrorResponse(exchange, 400, "Error: Redirects are only allowed to example.com with http/https scheme.");
            }
        } catch (URISyntaxException e) {
            // Malformed URL
            sendErrorResponse(exchange, 400, "Error: Malformed URL provided.");
        }
    }

    private static void sendErrorResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
        byte[] response = message.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
        exchange.sendResponseHeaders(statusCode, response.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response);
        }
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendErrorResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            String path = exchange.getRequestURI().getPath();
            String prefix = "/redirect/";
            if (path != null && path.startsWith(prefix)) {
                String encodedUrl = path.substring(prefix.length());
                String decodedUrl;
                try {
                     decodedUrl = URLDecoder.decode(encodedUrl, StandardCharsets.UTF_8.name());
                } catch (IllegalArgumentException e) {
                     sendErrorResponse(exchange, 400, "Invalid URL encoding.");
                     return;
                }
                
                redirect_invalid_url(exchange, decodedUrl);
            } else {
                sendErrorResponse(exchange, 404, "Not Found");
            }
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/redirect/", new RedirectHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on http://localhost:" + port);
        System.out.println("--- Test Cases (run in a separate terminal) ---");
        System.out.println("1. Valid HTTP redirect: curl -i -L \"http://localhost:8080/redirect/http%3A%2F%2Fexample.com\"");
        System.out.println("2. Valid HTTPS redirect with path: curl -i -L \"http://localhost:8080/redirect/https%3A%2F%2Fexample.com%2Fpath\"");
        System.out.println("3. Invalid domain: curl -i \"http://localhost:8080/redirect/http%3A%2F%2Fgoogle.com\"");
        System.out.println("4. Invalid scheme: curl -i \"http://localhost:8080/redirect/ftp%3A%2F%2Fexample.com\"");
        System.out.println("5. Malformed URL: curl -i \"http://localhost:8080/redirect/not-a-valid-url\"");
        System.out.println("-------------------------------------------------");
    }
}