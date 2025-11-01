import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse29 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        server.createContext("/", new RootHandler());
        server.createContext("/redirect", new RedirectHandler());
        
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Java server started on http://localhost:" + port);
        printTestCases(port);
    }

    private static void printTestCases(int port) {
        String baseUrl = "http://localhost:" + port;
        System.out.println("--- 5 Test Cases ---");
        System.out.println("Execute these commands in a separate terminal:");
        System.out.println("1. Valid Target: curl -I \"" + baseUrl + "/redirect?target=http://www.example.com\"");
        System.out.println("   Expected: 302 Redirect to http://www.example.com");
        System.out.println("2. Valid Subdomain: curl -I \"" + baseUrl + "/redirect?target=https://blog.example.com/article/1\"");
        System.out.println("   Expected: 302 Redirect to https://blog.example.com/article/1");
        System.out.println("3. Invalid Target: curl -I \"" + baseUrl + "/redirect?target=http://www.google.com\"");
        System.out.println("   Expected: 302 Redirect to /");
        System.out.println("4. Invalid TLD: curl -I \"" + baseUrl + "/redirect?target=http://example.community.net\"");
        System.out.println("   Expected: 302 Redirect to /");
        System.out.println("5. Missing Target: curl -I \"" + baseUrl + "/redirect\"");
        System.out.println("   Expected: 302 Redirect to /");
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "Welcome! This is the default page.";
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    static class RedirectHandler implements HttpHandler {
        private static final Pattern TARGET_PATTERN = Pattern.compile(".*example\\.com.*");

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            Map<String, String> params = queryToMap(exchange.getRequestURI().getQuery());
            String target = params.get("target");

            if (target != null && TARGET_PATTERN.matcher(target).matches()) {
                exchange.getResponseHeaders().add("Location", target);
            } else {
                exchange.getResponseHeaders().add("Location", "/");
            }
            exchange.sendResponseHeaders(302, -1); // 302 Found for temporary redirect
        }

        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null) {
                return result;
            }
            for (String param : query.split("&")) {
                String[] entry = param.split("=", 2);
                if (entry.length > 1) {
                    result.put(URLDecoder.decode(entry[0], StandardCharsets.UTF_8), URLDecoder.decode(entry[1], StandardCharsets.UTF_8));
                } else {
                    result.put(URLDecoder.decode(entry[0], StandardCharsets.UTF_8), "");
                }
            }
            return result;
        }
    }
}