import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse29 {
    private static final Pattern ALLOW_RE = Pattern.compile("(?i)^https?://[^\\s]*example\\.com[^\\s]*$");
    private static final int MAX_LEN = 2048;

    public static void main(String[] args) throws Exception {
        // 5 test cases
        String[] tests = new String[]{
                "https://example.com",
                "http://sub.example.com/path?x=1",
                "https://notexample.com",
                "https://example.com.evil.com/attack",
                "/local/path?target=https://example.com"
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println("Java Test " + (i + 1) + ": input=" + tests[i] + " -> redirect=" + evaluateRedirect(tests[i]));
        }

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/go", new RedirectHandler());
        server.setExecutor(null);
        System.out.println("Java server listening on http://127.0.0.1:8080/go?target=...");
        server.start();
    }

    public static String evaluateRedirect(String target) {
        if (target == null) return "/";
        target = target.trim();
        if (target.length() == 0 || target.length() > MAX_LEN) return "/";
        if (containsCtl(target)) return "/";
        if (!ALLOW_RE.matcher(target).matches()) return "/";
        return target;
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                Map<String, String> q = parseQuery(exchange.getRequestURI().getRawQuery());
                String target = q.get("target");
                String location = evaluateRedirect(target);

                Headers h = exchange.getResponseHeaders();
                h.set("Location", location);
                h.set("Content-Type", "text/plain; charset=utf-8");
                h.set("Cache-Control", "no-store");
                h.set("Content-Security-Policy", "default-src 'none'; frame-ancestors 'none'");

                byte[] body = ("Redirecting to: " + location + "\n").getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(302, body.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(body);
                }
            } finally {
                exchange.close();
            }
        }
    }

    private static Map<String, String> parseQuery(String raw) throws IOException {
        Map<String, String> map = new HashMap<>();
        if (raw == null || raw.isEmpty()) return map;
        String[] parts = raw.split("&");
        for (String p : parts) {
            int idx = p.indexOf('=');
            String k = idx >= 0 ? p.substring(0, idx) : p;
            String v = idx >= 0 ? p.substring(idx + 1) : "";
            k = URLDecoder.decode(k, StandardCharsets.UTF_8.name());
            v = URLDecoder.decode(v, StandardCharsets.UTF_8.name());
            map.put(k, v);
        }
        return map;
    }

    private static boolean containsCtl(String s) {
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '\r' || c == '\n') return true;
            if (c < 0x20 && c != '\t') return true;
            if (c == 0x7F) return true;
        }
        return false;
    }
}