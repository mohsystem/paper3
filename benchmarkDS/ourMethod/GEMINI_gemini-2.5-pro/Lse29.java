import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse29 {

    // A regex that matches URLs where the host contains "example.com".
    // This is safer than a simple substring check on the whole URL.
    private static final String ALLOWED_DOMAIN_REGEX = "^https?://[^/]*example\\.com.*";
    private static final Pattern DOMAIN_PATTERN = Pattern.compile(ALLOWED_DOMAIN_REGEX);

    /**
     * Determines the redirect URL based on a target parameter.
     *
     * @param target The target URL to validate.
     * @return The validated target URL if it matches the regex, otherwise "/".
     */
    public static String getRedirectUrl(String target) {
        if (target != null && !target.isEmpty()) {
            if (DOMAIN_PATTERN.matcher(target).matches()) {
                return target;
            }
        }
        return "/";
    }

    public static void main(String[] args) throws IOException {
        runTests();
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new RedirectHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Test with a browser, e.g., http://localhost:8080/?target=http://example.com");
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String target = null;
            String query = exchange.getRequestURI().getRawQuery();
            if (query != null) {
                Map<String, String> params = queryToMap(query);
                target = params.get("target");
            }
            
            String redirectUrl = getRedirectUrl(target);
            
            if ("/".equals(redirectUrl)) {
                // If the target is invalid or missing, serve a 200 OK page
                // to avoid a redirect loop.
                String response = "Welcome. Provide a valid 'target' parameter pointing to a URL " +
                                  "at example.com to be redirected.";
                exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
                exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            } else {
                // If the target is valid, perform the redirect.
                exchange.getResponseHeaders().set("Location", redirectUrl);
                exchange.sendResponseHeaders(302, -1); // 302 Found
                exchange.close();
            }
        }

        private Map<String, String> queryToMap(String query) {
            if (query == null || query.isEmpty()) {
                return Collections.emptyMap();
            }
            Map<String, String> result = new HashMap<>();
            for (String param : query.split("&")) {
                String[] entry = param.split("=", 2);
                try {
                    String key = URLDecoder.decode(entry[0], StandardCharsets.UTF_8.name());
                    String value = entry.length > 1 ? URLDecoder.decode(entry[1], StandardCharsets.UTF_8.name()) : "";
                    result.put(key, value);
                } catch (UnsupportedEncodingException e) {
                    // Should not happen with UTF-8
                    System.err.println("Error decoding URL parameter: " + e.getMessage());
                }
            }
            return result;
        }
    }

    public static void runTests() {
        System.out.println("\n--- Running 5 Test Cases ---");
        
        // Test Case 1: Valid URL
        String test1 = "http://example.com";
        System.out.println("1. (Valid) " + test1 + " -> " + getRedirectUrl(test1));

        // Test Case 2: Valid URL with subdomain and path
        String test2 = "http://sub.example.com/some/path";
        System.out.println("2. (Valid) " + test2 + " -> " + getRedirectUrl(test2));
        
        // Test Case 3: Invalid domain
        String test3 = "http://another-domain.com";
        System.out.println("3. (Invalid) " + test3 + " -> " + getRedirectUrl(test3));

        // Test Case 4: Invalid URL with tricky parameter
        String test4 = "http://evil.com/?q=example.com";
        System.out.println("4. (Invalid) " + test4 + " -> " + getRedirectUrl(test4));
        
        // Test Case 5: Null target
        System.out.println("5. (Null target) null -> " + getRedirectUrl(null));

        System.out.println("--- Test Cases Finished ---\n");
    }
}