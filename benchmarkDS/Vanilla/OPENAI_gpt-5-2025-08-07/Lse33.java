import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;
import java.util.Map;

public class Lse33 {

    public static class Result {
        public final boolean ok;
        public final int status;
        public final String location;
        public final String message;

        public Result(boolean ok, int status, String location, String message) {
            this.ok = ok;
            this.status = status;
            this.location = location;
            this.message = message;
        }
    }

    public static Result redirect_invalid_url(String url) {
        if (url == null || url.trim().isEmpty()) {
            return new Result(false, 400, null, "Missing url parameter");
        }
        try {
            URI uri = new URI(url);
            String scheme = uri.getScheme();
            String host = uri.getHost();
            if (scheme == null || host == null) {
                return new Result(false, 400, null, "Invalid URL");
            }
            if (!scheme.equalsIgnoreCase("http") && !scheme.equalsIgnoreCase("https")) {
                return new Result(false, 400, null, "Unsupported scheme");
            }
            if (host.equalsIgnoreCase("example.com")) {
                return new Result(true, 302, url, "Redirecting");
            } else {
                return new Result(false, 400, null, "Invalid domain");
            }
        } catch (Exception e) {
            return new Result(false, 400, null, "Invalid URL");
        }
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new LinkedHashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] pairs = query.split("&");
        for (String p : pairs) {
            int idx = p.indexOf('=');
            if (idx >= 0) {
                String key = URLDecoder.decode(p.substring(0, idx), StandardCharsets.UTF_8);
                String value = URLDecoder.decode(p.substring(idx + 1), StandardCharsets.UTF_8);
                map.put(key, value);
            } else {
                String key = URLDecoder.decode(p, StandardCharsets.UTF_8);
                map.put(key, "");
            }
        }
        return map;
    }

    private static class GoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                String body = "Method Not Allowed";
                exchange.sendResponseHeaders(405, body.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(body.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }

            Map<String, String> params = parseQuery(exchange.getRequestURI().getRawQuery());
            String url = params.get("url");
            Result res = redirect_invalid_url(url);
            if (res.ok) {
                Headers headers = exchange.getResponseHeaders();
                headers.add("Location", res.location);
                String body = "Redirecting to " + res.location;
                exchange.sendResponseHeaders(302, body.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(body.getBytes(StandardCharsets.UTF_8));
                }
            } else {
                String body = res.message;
                exchange.sendResponseHeaders(res.status, body.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(body.getBytes(StandardCharsets.UTF_8));
                }
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        String[] tests = new String[] {
                "https://example.com/path",
                "http://example.com",
                "https://sub.example.com",
                "https://example.org",
                "not a url"
        };
        for (String t : tests) {
            Result r = redirect_invalid_url(t);
            System.out.println("Test: " + t + " -> ok=" + r.ok + " status=" + r.status + " message=" + (r.ok ? r.location : r.message));
        }

        // Start simple HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/go", new GoHandler());
        server.setExecutor(null);
        System.out.println("Server running at http://127.0.0.1:8080/go?url=https%3A%2F%2Fexample.com%2F");
        server.start();
    }
}