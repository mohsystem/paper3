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
import java.util.Locale;

public class Lse33 {

    public static final int MAX_URL_LENGTH = 2048;
    public static final int MAX_QUERY_LENGTH = 4096;

    public static final class RedirectResult {
        public final int statusCode; // 302 for redirect, 400 for error
        public final String location; // set when 302
        public final String body;     // set when 400

        public RedirectResult(int statusCode, String location, String body) {
            this.statusCode = statusCode;
            this.location = location;
            this.body = body;
        }
    }

    // Core function: accepts input as a parameter and returns output
    public static RedirectResult redirect_invalid_url(String url) {
        if (url == null) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        String trimmed = url.trim();
        if (trimmed.isEmpty() || trimmed.length() > MAX_URL_LENGTH) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        // Reject CR/LF in the input to prevent header injection later
        if (trimmed.indexOf('\r') >= 0 || trimmed.indexOf('\n') >= 0) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        URI u;
        try {
            u = new URI(trimmed);
        } catch (Exception e) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        String scheme = u.getScheme();
        if (scheme == null) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        String schemeLower = scheme.toLowerCase(Locale.ROOT);
        if (!schemeLower.equals("http") && !schemeLower.equals("https")) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        if (u.getRawUserInfo() != null) { // reject userinfo
            return new RedirectResult(400, null, "Invalid URL");
        }
        String host = u.getHost();
        if (host == null) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        if (!"example.com".equals(host.toLowerCase(Locale.ROOT))) {
            return new RedirectResult(400, null, "Invalid URL");
        }
        // Sanitize location for safety (strip CR/LF)
        String safeLocation = trimmed.replace("\r", "").replace("\n", "");
        return new RedirectResult(302, safeLocation, null);
    }

    // Simple web server
    static class GoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendPlain(exchange, 405, "Method Not Allowed");
                    return;
                }
                URI reqUri = exchange.getRequestURI();
                if (!"/go".equals(reqUri.getPath())) {
                    sendPlain(exchange, 404, "Not Found");
                    return;
                }
                String rawQuery = reqUri.getRawQuery();
                if (rawQuery == null || rawQuery.isEmpty() || rawQuery.length() > MAX_QUERY_LENGTH) {
                    sendPlain(exchange, 400, "Invalid URL");
                    return;
                }
                String urlParam = null;
                String[] parts = rawQuery.split("&");
                for (String p : parts) {
                    int eq = p.indexOf('=');
                    String key = eq >= 0 ? p.substring(0, eq) : p;
                    if ("url".equals(key)) {
                        String value = eq >= 0 ? p.substring(eq + 1) : "";
                        urlParam = URLDecoder.decode(value, StandardCharsets.UTF_8);
                        break;
                    }
                }
                if (urlParam == null) {
                    sendPlain(exchange, 400, "Invalid URL");
                    return;
                }
                RedirectResult res = redirect_invalid_url(urlParam);
                if (res.statusCode == 302) {
                    Headers h = exchange.getResponseHeaders();
                    h.set("Location", res.location);
                    exchange.sendResponseHeaders(302, -1); // no body
                } else {
                    sendPlain(exchange, res.statusCode, "Invalid URL");
                }
            } catch (Exception e) {
                // Fail closed
                sendPlain(exchange, 400, "Invalid URL");
            }
        }

        private void sendPlain(HttpExchange ex, int code, String msg) throws IOException {
            byte[] data = msg.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            ex.sendResponseHeaders(code, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }
    }

    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/go", new GoHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on http://127.0.0.1:" + port + "/go?url=http%3A%2F%2Fexample.com%2F");
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for the core function
        String[] tests = new String[] {
                "http://example.com/",
                "https://example.com/path?x=1",
                "https://sub.example.com/",
                "ftp://example.com/",
                "https://example.com/\r\nX-Injected: header"
        };
        for (String t : tests) {
            RedirectResult r = redirect_invalid_url(t);
            if (r.statusCode == 302) {
                System.out.println("OK 302 -> " + r.location);
            } else {
                System.out.println("ERR " + r.statusCode);
            }
        }

        // Optionally start the web server if "serve" argument is passed
        if (args.length > 0 && "serve".equalsIgnoreCase(args[0])) {
            startServer(8080);
            // Keep alive
            Thread.currentThread().join();
        }
    }
}