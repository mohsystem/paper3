import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;
import java.util.Map;
import java.util.HashMap;

public class Lse29 {

    public static void main(String[] args) throws IOException {
        int port = 8000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new RedirectHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Test with URLs like:");
        System.out.println("http://localhost:8000/?target=https://safe.example.com/path");
        System.out.println("http://localhost:8000/?target=https://www.google.com");
    }

    static class RedirectHandler implements HttpHandler {
        // Secure regex to validate the target URL.
        // It ensures the protocol is http/https, the domain is example.com or a subdomain,
        // and prevents user-info pass-through (e.g., http://example.com@evil.com).
        private static final Pattern URL_PATTERN = Pattern.compile(
                "^https?://(?!.*@)([a-zA-Z0-9-]+\\.)*example\\.com(/.*)?$"
        );

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = parseQuery(query);
            String target = params.get("target");

            String redirectTo = "/";
            if (target != null && !target.isEmpty()) {
                if (URL_PATTERN.matcher(target).matches()) {
                    redirectTo = target;
                }
            }

            exchange.getResponseHeaders().set("Location", redirectTo);
            exchange.sendResponseHeaders(302, -1); // 302 Found for redirection
            try (OutputStream os = exchange.getResponseBody()) {
                // No body needed for a redirect
            }
        }

        private Map<String, String> parseQuery(String query) {
            Map<String, String> params = new HashMap<>();
            if (query == null) {
                return params;
            }
            try {
                String[] pairs = query.split("&");
                for (String pair : pairs) {
                    int idx = pair.indexOf("=");
                    if (idx > 0 && idx < pair.length() - 1) {
                         String key = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8.name());
                         String value = URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8.name());
                         params.put(key, value);
                    }
                }
            } catch (java.io.UnsupportedEncodingException e) {
                // Should not happen with UTF-8
                System.err.println("Error decoding URL: " + e.getMessage());
            }
            return params;
        }
    }
}