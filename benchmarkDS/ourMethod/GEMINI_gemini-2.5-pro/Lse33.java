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
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

public class Lse33 {

    private static final String ALLOWED_DOMAIN = "example.com";

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/redirect", new RedirectHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("1. Valid Redirect: curl -v \"http://localhost:8080/redirect?url=http%3A%2F%2Fexample.com%2Fsome%2Fpath\"");
        System.out.println("2. Valid Redirect (HTTPS): curl -v \"http://localhost:8080/redirect?url=https%3A%2F%2Fexample.com\"");
        System.out.println("3. Invalid Domain: curl -v \"http://localhost:8080/redirect?url=http%3A%2F%2Fgoogle.com\"");
        System.out.println("4. Invalid Subdomain Attack: curl -v \"http://localhost:8080/redirect?url=http%3A%2F%2Fexample.com.evil.com\"");
        System.out.println("5. Malformed URL: curl -v \"http://localhost:8080/redirect?url=not_a_valid_url\"");
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            String query = exchange.getRequestURI().getRawQuery();
            Map<String, String> params = parseQuery(query);
            String urlToRedirect = params.get("url");

            redirectInvalidUrl(exchange, urlToRedirect);
        }
    }

    public static void redirectInvalidUrl(HttpExchange exchange, String urlToRedirect) throws IOException {
        if (urlToRedirect == null || urlToRedirect.isEmpty()) {
            sendResponse(exchange, 400, "URL parameter is missing.");
            return;
        }

        try {
            // URL Decoding is handled by parseQuery, but let's ensure it's clean if used directly.
            // The parameter is already decoded by our simple parser.
            URI uri = new URI(urlToRedirect);

            // Validate the scheme to prevent redirects to non-http protocols like 'javascript:' or 'file:'.
            String scheme = uri.getScheme();
            if (scheme == null || (!scheme.equalsIgnoreCase("http") && !scheme.equalsIgnoreCase("https"))) {
                sendResponse(exchange, 400, "Invalid URL scheme.");
                return;
            }

            String host = uri.getHost();
            if (host != null && host.equalsIgnoreCase(ALLOWED_DOMAIN)) {
                exchange.getResponseHeaders().add("Location", uri.toString());
                exchange.sendResponseHeaders(302, -1); // 302 Found for redirect
            } else {
                sendResponse(exchange, 400, "Invalid URL or domain not allowed.");
            }
        } catch (URISyntaxException e) {
            sendResponse(exchange, 400, "Invalid URL format.");
        }
    }

    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        exchange.sendResponseHeaders(statusCode, responseBytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(responseBytes);
        }
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> params = new HashMap<>();
        if (query != null && !query.isEmpty()) {
            for (String param : query.split("&")) {
                String[] pair = param.split("=", 2);
                if (pair.length > 1) {
                    try {
                        String key = URLDecoder.decode(pair[0], StandardCharsets.UTF_8.name());
                        String value = URLDecoder.decode(pair[1], StandardCharsets.UTF_8.name());
                        params.put(key, value);
                    } catch (java.io.UnsupportedEncodingException e) {
                        // This should not happen with UTF-8
                    }
                }
            }
        }
        return params;
    }
}