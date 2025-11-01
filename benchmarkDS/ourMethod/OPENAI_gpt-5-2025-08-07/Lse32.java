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
import java.util.concurrent.Executors;

public class Lse32 {

    // Result type for validation
    static class RedirectResult {
        final boolean valid;
        final String message; // error message if invalid
        final String target;  // sanitized redirect target if valid

        RedirectResult(boolean valid, String message, String target) {
            this.valid = valid;
            this.message = message;
            this.target = target;
        }
    }

    // Validation function per requirements
    public static RedirectResult redirect_invalid_url(String rawUrl) {
        if (rawUrl == null) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        if (rawUrl.length() == 0 || rawUrl.length() > 2048) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        // Reject control characters and header injection
        for (int i = 0; i < rawUrl.length(); i++) {
            char c = rawUrl.charAt(i);
            if (c <= 0x1F || c == 0x7F) {
                return new RedirectResult(false, "Invalid URL", null);
            }
        }
        final URI uri;
        try {
            uri = new URI(rawUrl);
        } catch (Exception e) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        String scheme = uri.getScheme();
        if (scheme == null) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        String s = scheme.toLowerCase(Locale.ROOT);
        if (!("http".equals(s) || "https".equals(s))) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        if (uri.getUserInfo() != null) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        String host = uri.getHost();
        if (host == null) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        if (!"example.com".equalsIgnoreCase(host)) {
            return new RedirectResult(false, "Invalid URL", null);
        }
        // Passed validation
        return new RedirectResult(true, "OK", uri.toString());
    }

    // HTTP handler for /redirect/<url>
    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendText(exchange, 405, "Method Not Allowed");
                    return;
                }
                URI reqUri = exchange.getRequestURI();
                String path = reqUri.getPath();
                String prefix = "/redirect/";
                if (path == null || !path.startsWith(prefix)) {
                    sendText(exchange, 404, "Not Found");
                    return;
                }
                String encodedPart = path.substring(prefix.length());
                if (encodedPart.isEmpty()) {
                    sendText(exchange, 400, "Invalid URL");
                    return;
                }
                String decodedUrl;
                try {
                    // Decode just once; treat '+' literally to avoid form-style issues
                    decodedUrl = URLDecoder.decode(encodedPart, StandardCharsets.UTF_8.name()).replace("+", "%2B");
                    decodedUrl = URLDecoder.decode(encodedPart, StandardCharsets.UTF_8.name());
                } catch (IllegalArgumentException e) {
                    sendText(exchange, 400, "Invalid URL");
                    return;
                }
                RedirectResult res = redirect_invalid_url(decodedUrl);
                if (!res.valid) {
                    sendText(exchange, 400, "Invalid URL");
                    return;
                }
                Headers h = exchange.getResponseHeaders();
                h.set("Location", res.target);
                byte[] body = "Redirecting...\n".getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(302, body.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(body);
                }
            } catch (Exception ex) {
                // Generic error without leaking details
                try {
                    sendText(exchange, 500, "Server Error");
                } catch (Exception ignored) {
                }
            }
        }
    }

    private static void sendText(HttpExchange ex, int code, String msg) throws IOException {
        byte[] data = (msg + "\n").getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        ex.sendResponseHeaders(code, data.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(data);
        }
    }

    public static void main(String[] args) throws Exception {
        // Start simple HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/redirect", new RedirectHandler()); // match /redirect and subpaths
        server.createContext("/redirect/", new RedirectHandler());
        server.setExecutor(Executors.newFixedThreadPool(2));
        server.start();
        System.out.println("Server listening on http://127.0.0.1:8080");

        // Five test cases for the function
        String[] tests = new String[]{
                "https://example.com/path",
                "http://example.com",
                "https://evil.com/",
                "https://example.com:443/ok",
                "https://user@example.com/"
        };
        for (String t : tests) {
            RedirectResult r = redirect_invalid_url(t);
            if (r.valid) {
                System.out.println("[VALID] " + t + " -> " + r.target);
            } else {
                System.out.println("[INVALID] " + t + " -> " + r.message);
            }
        }

        // Stop the server after tests to allow program exit in environments without long-running tasks
        server.stop(0);
    }
}