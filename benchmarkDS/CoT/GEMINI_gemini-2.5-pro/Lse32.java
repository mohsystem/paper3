import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.URL;

public class Lse32 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/redirect/", new RedirectHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Java server started on port " + port);
        System.out.println("Test with the following URLs:");
        System.out.println("1. Valid: http://localhost:8080/redirect/http://example.com");
        System.out.println("2. Valid with path: http://localhost:8080/redirect/https://example.com/some/path");
        System.out.println("3. Invalid Host: http://localhost:8080/redirect/http://google.com");
        System.out.println("4. Tricky URL (invalid): http://localhost:8080/redirect/http://example.com@evil.com");
        System.out.println("5. Malformed URL: http://localhost:8080/redirect/not-a-valid-url");
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            // Expected path format: /redirect/<url>
            if (path.length() <= "/redirect/".length()) {
                sendErrorResponse(exchange, 400, "URL parameter is missing.");
                return;
            }
            String urlToRedirect = path.substring("/redirect/".length());
            redirectInvalidUrl(exchange, urlToRedirect);
        }

        private void redirectInvalidUrl(HttpExchange exchange, String urlString) throws IOException {
            try {
                // Use java.net.URL for robust parsing
                URL url = new URL(urlString);
                String host = url.getHost();

                // Security Check: Only allow redirects to "example.com"
                if ("example.com".equalsIgnoreCase(host)) {
                    // Safe to redirect
                    exchange.getResponseHeaders().add("Location", urlString);
                    exchange.sendResponseHeaders(302, -1); // 302 Found for temporary redirect
                } else {
                    // Disallowed host
                    sendErrorResponse(exchange, 400, "Invalid URL. Only redirects to example.com are allowed.");
                }
            } catch (MalformedURLException e) {
                // The provided string was not a valid URL
                sendErrorResponse(exchange, 400, "Malformed URL provided.");
            }
        }

        private void sendErrorResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            exchange.sendResponseHeaders(statusCode, message.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(message.getBytes());
            os.close();
        }
    }
}