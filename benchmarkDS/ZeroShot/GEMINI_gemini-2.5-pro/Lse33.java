import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse33 {

    /**
     * Checks if the URL has the domain "example.com" and performs a redirect.
     *
     * @param exchange The HttpExchange object to handle the request and response.
     * @param targetUrl The URL string to validate and redirect to.
     * @throws IOException
     */
    public static void redirect_invalid_url(HttpExchange exchange, String targetUrl) throws IOException {
        String responseMessage;
        int statusCode;
        
        if (targetUrl == null || targetUrl.trim().isEmpty()) {
            statusCode = 400; // Bad Request
            responseMessage = "Error: URL parameter is missing.";
            exchange.sendResponseHeaders(statusCode, responseMessage.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseMessage.getBytes(StandardCharsets.UTF_8));
            }
            return;
        }

        try {
            // Use URI class for robust parsing to prevent parsing exploits
            URI uri = new URI(targetUrl);
            String host = uri.getHost();

            // Secure check: only allow redirection to the exact "example.com" domain.
            // This prevents open redirect vulnerabilities.
            if (host != null && host.equalsIgnoreCase("example.com")) {
                // Set Location header for redirection
                exchange.getResponseHeaders().set("Location", targetUrl);
                // 302 Found is a common status code for temporary redirects
                exchange.sendResponseHeaders(302, -1); // -1 means no response body
            } else {
                statusCode = 400; // Bad Request
                responseMessage = "Error: Invalid domain. Redirection is only allowed to example.com.";
                exchange.sendResponseHeaders(statusCode, responseMessage.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(responseMessage.getBytes(StandardCharsets.UTF_8));
                }
            }
        } catch (URISyntaxException e) {
            statusCode = 400; // Bad Request
            responseMessage = "Error: Malformed URL provided.";
            exchange.sendResponseHeaders(statusCode, responseMessage.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseMessage.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = queryToMap(query);
            String urlToRedirect = params.get("url");
            redirect_invalid_url(exchange, urlToRedirect);
        }

        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null) {
                return result;
            }
            for (String param : query.split("&")) {
                String[] entry = param.split("=");
                if (entry.length > 1) {
                    result.put(entry[0], entry[1]);
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/redirect", new RedirectHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Use a browser or curl to test the /redirect endpoint.");
        System.out.println("\n--- Test Cases ---");
        System.out.println("1. Valid URL (should redirect):");
        System.out.println("   curl -I \"http://localhost:8080/redirect?url=http://example.com/test\"");
        System.out.println("\n2. Invalid Domain (should return 400 error):");
        System.out.println("   curl \"http://localhost:8080/redirect?url=http://google.com\"");
        System.out.println("\n3. Invalid Subdomain Exploit Attempt (should return 400 error):");
        System.out.println("   curl \"http://localhost:8080/redirect?url=http://example.com.malicious.com\"");
        System.out.println("\n4. Missing URL parameter (should return 400 error):");
        System.out.println("   curl \"http://localhost:8080/redirect\"");
        System.out.println("\n5. Malformed URL (should return 400 error):");
        System.out.println("   curl \"http://localhost:8080/redirect?url=not_a_valid_url\"");
    }
}