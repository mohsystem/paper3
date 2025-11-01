import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.regex.Pattern;

public class Lse29 {
    private static final Pattern ALLOW_PATTERN = Pattern.compile(".*example\\.com.*");

    public static boolean matchesAllowed(String target) {
        if (target == null) return false;
        return ALLOW_PATTERN.matcher(target).matches();
    }

    public static String resolveRedirect(String target) {
        if (target == null) target = "";
        return matchesAllowed(target) ? target : "/";
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        for (String part : query.split("&")) {
            int idx = part.indexOf('=');
            String key = idx >= 0 ? part.substring(0, idx) : part;
            String val = idx >= 0 ? part.substring(idx + 1) : "";
            try {
                key = URLDecoder.decode(key, StandardCharsets.UTF_8);
                val = URLDecoder.decode(val, StandardCharsets.UTF_8);
            } catch (Exception ignored) {}
            map.put(key, val);
        }
        return map;
    }

    private static void sendRedirect(HttpExchange ex, String location) throws IOException {
        Headers h = ex.getResponseHeaders();
        h.set("Location", location);
        ex.sendResponseHeaders(302, -1);
        ex.close();
    }

    private static void sendText(HttpExchange ex, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        ex.sendResponseHeaders(200, bytes.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(bytes);
        }
    }

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 0), 0);
        server.createContext("/go", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String query = exchange.getRequestURI().getRawQuery();
                Map<String, String> qs = parseQuery(query);
                String target = qs.get("target");
                String location = resolveRedirect(target);
                sendRedirect(exchange, location);
            }
        });
        server.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                sendText(exchange, "Home");
            }
        });
        server.setExecutor(null);
        server.start();
        int port = server.getAddress().getPort();

        // 5 test cases using HTTP client (no auto-follow redirects)
        List<String> tests = Arrays.asList(
                "http://example.com/page",
                "https://sub.example.com",
                "http://evil.com/?next=example.com",
                "https://another.com/path",
                null // missing target
        );
        for (int i = 0; i < tests.size(); i++) {
            String t = tests.get(i);
            String path;
            if (t == null) {
                path = "/go";
            } else {
                path = "/go?target=" + URLEncoder.encode(t, StandardCharsets.UTF_8);
            }
            URL url = new URL("http://127.0.0.1:" + port + path);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setInstanceFollowRedirects(false);
            conn.setConnectTimeout(3000);
            conn.setReadTimeout(3000);
            conn.getResponseCode(); // trigger request
            String loc = conn.getHeaderField("Location");
            System.out.println("Test " + (i + 1) + ": target=" + (t == null ? "(missing)" : t) + " -> redirect to " + loc);
            conn.disconnect();
        }

        server.stop(0);
    }
}