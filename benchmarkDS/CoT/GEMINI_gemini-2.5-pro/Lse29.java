import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Map;

public class Lse29 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new RedirectHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("1. Valid Main Domain: http://localhost:8080/?target=http://example.com/home");
        System.out.println("   Expected: Redirect to http://example.com/home");
        System.out.println("2. Valid Subdomain: http://localhost:8080/?target=https://sub.example.com/path?q=1");
        System.out.println("   Expected: Redirect to https://sub.example.com/path?q=1");
        System.out.println("3. Invalid Domain: http://localhost:8080/?target=http://evilexample.com");
        System.out.println("   Expected: Redirect to /");
        System.out.println("4. Tricky Invalid Domain: http://localhost:8080/?target=http://evil.com?p=example.com");
        System.out.println("   Expected: Redirect to /");
        System.out.println("5. No Target or Malformed: http://localhost:8080/ or http://localhost:8080/?target=foo");
        System.out.println("   Expected: Redirect to /");
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = queryToMap(query);
            String target = params.get("target");

            String redirectTo = "/";
            if (isValidTarget(target)) {
                redirectTo = target;
            }

            exchange.getResponseHeaders().set("Location", redirectTo);
            exchange.sendResponseHeaders(302, -1); // 302 Found for redirect
            try (OutputStream os = exchange.getResponseBody()) {
                // No body for redirect
            }
        }

        private boolean isValidTarget(String targetUrl) {
            if (targetUrl == null || targetUrl.trim().isEmpty()) {
                return false;
            }
            try {
                URI uri = new URI(targetUrl);
                String host = uri.getHost();

                if (host == null) {
                    return false; // Not a valid absolute URL with a host
                }

                // Secure check: host must be "example.com" or a subdomain
                return host.equalsIgnoreCase("example.com") || host.toLowerCase().endsWith(".example.com");

            } catch (URISyntaxException e) {
                return false; // Malformed URL
            }
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
}