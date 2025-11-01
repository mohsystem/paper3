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

    public static class RedirectResult {
        public final boolean ok;
        public final String target;
        public final String error;

        public RedirectResult(boolean ok, String target, String error) {
            this.ok = ok;
            this.target = target;
            this.error = error;
        }
    }

    public static RedirectResult redirect_invalid_url(String rawUrl) {
        if (rawUrl == null) {
            return new RedirectResult(false, null, "Missing URL parameter.");
        }
        if (rawUrl.length() > 2048) {
            return new RedirectResult(false, null, "URL too long.");
        }
        // Basic CRLF/header-injection guard
        if (rawUrl.indexOf('\r') >= 0 || rawUrl.indexOf('\n') >= 0) {
            return new RedirectResult(false, null, "Invalid characters in URL.");
        }

        try {
            URI uri = new URI(rawUrl);
            String scheme = uri.getScheme();
            if (scheme == null) return new RedirectResult(false, null, "URL must include a scheme.");
            String s = scheme.toLowerCase();
            if (!s.equals("http") && !s.equals("https")) {
                return new RedirectResult(false, null, "Unsupported scheme.");
            }

            String host = uri.getHost();
            if (host == null) {
                // Workaround for some URIs: attempt to re-parse via toASCIIString if host is null
                URI normalized = new URI(uri.toString());
                host = normalized.getHost();
            }
            if (host == null) {
                return new RedirectResult(false, null, "Invalid host.");
            }
            if (!"example.com".equalsIgnoreCase(host)) {
                return new RedirectResult(false, null, "URL must be on example.com.");
            }

            // Disallow userinfo for safety
            if (uri.getUserInfo() != null) {
                return new RedirectResult(false, null, "Userinfo not allowed in URL.");
            }

            // Normalize minimal: ensure path not null
            String path = uri.getRawPath();
            if (path == null || path.isEmpty()) path = "/";

            URI safe = new URI(
                    s,
                    null, // no userinfo
                    host.toLowerCase(),
                    uri.getPort(),
                    path,
                    uri.getRawQuery(),
                    uri.getRawFragment()
            );
            return new RedirectResult(true, safe.toString(), null);

        } catch (Exception e) {
            return new RedirectResult(false, null, "Malformed URL.");
        }
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> result = new LinkedHashMap<>();
        if (query == null || query.isEmpty()) return result;
        for (String pair : query.split("&")) {
            int idx = pair.indexOf('=');
            String key = idx > 0 ? pair.substring(0, idx) : pair;
            String value = idx > 0 ? pair.substring(idx + 1) : "";
            try {
                key = URLDecoder.decode(key, StandardCharsets.UTF_8.name());
                value = URLDecoder.decode(value, StandardCharsets.UTF_8.name());
            } catch (Exception ignored) {}
            result.put(key, value);
        }
        return result;
    }

    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/go", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    byte[] body = "Method Not Allowed".getBytes(StandardCharsets.UTF_8);
                    exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
                    exchange.sendResponseHeaders(405, body.length);
                    try (OutputStream os = exchange.getResponseBody()) { os.write(body); }
                    return;
                }
                URI reqUri = exchange.getRequestURI();
                Map<String, String> params = parseQuery(reqUri.getRawQuery());
                String url = params.get("url");
                RedirectResult res = redirect_invalid_url(url);
                if (res.ok) {
                    exchange.getResponseHeaders().add("Location", res.target);
                    exchange.getResponseHeaders().add("Cache-Control", "no-store");
                    exchange.sendResponseHeaders(302, -1);
                } else {
                    byte[] body = ("Error: " + (res.error == null ? "Invalid request" : res.error)).getBytes(StandardCharsets.UTF_8);
                    exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
                    exchange.sendResponseHeaders(400, body.length);
                    try (OutputStream os = exchange.getResponseBody()) { os.write(body); }
                }
            }
        });
        server.setExecutor(null);
        server.start();
    }

    public static void main(String[] args) throws Exception {
        // Start web server on port 8080
        startServer(8080);

        // Five test cases
        String[] tests = new String[] {
                "https://example.com/",
                "http://example.com/path?x=1",
                "https://sub.example.com/",
                "https://example.com.evil.com/",
                "https://example.com:8080/path#frag"
        };
        for (String t : tests) {
            RedirectResult r = redirect_invalid_url(t);
            if (r.ok) {
                System.out.println("OK -> " + r.target);
            } else {
                System.out.println("ERR -> " + r.error);
            }
        }
        System.out.println("Java server running on http://127.0.0.1:8080/go?url=<encoded_url>");
    }
}