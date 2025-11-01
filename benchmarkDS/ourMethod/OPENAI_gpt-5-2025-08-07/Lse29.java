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
import java.util.regex.Pattern;

public class Lse29 {

    private static final int PORT = 8080;
    private static final int MAX_PARAM_LEN = 2048;
    private static final int MAX_QUERY_LEN = 4096;
    private static final Pattern ALLOW_RE = Pattern.compile("example\\.com", Pattern.CASE_INSENSITIVE);

    public static void main(String[] args) throws Exception {
        // 5 test cases for decideRedirect
        String[] tests = new String[] {
                "http://example.com/path",
                "https://sub.example.com/",
                "http://evil.com/?q=example.com",
                "/local/path",
                "javascript:example.com"
        };
        for (String t : tests) {
            String r = decideRedirect(t);
            System.out.println("Input: " + String.valueOf(t) + " -> Redirect: " + r);
        }

        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", PORT), 0);
        server.createContext("/", new RedirectHandler());
        server.setExecutor(null);
        System.out.println("Java server listening on http://127.0.0.1:" + PORT + " - try /?target=http://example.com");
        server.start();
    }

    // Core decision function: accepts input and returns redirect target
    public static String decideRedirect(String target) {
        if (target == null) {
            return "/";
        }
        String cleaned = stripCRLF(target);
        if (cleaned.length() > MAX_PARAM_LEN) {
            return "/";
        }
        if (ALLOW_RE.matcher(cleaned).find()) {
            return cleaned;
        }
        return "/";
    }

    private static String stripCRLF(String s) {
        StringBuilder sb = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c != '\r' && c != '\n') {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    private static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed");
                    return;
                }
                URI uri = exchange.getRequestURI();
                String query = uri.getRawQuery();
                String targetParam = parseTarget(query);
                String location = decideRedirect(targetParam);
                Headers headers = exchange.getResponseHeaders();
                headers.set("Location", location);
                headers.set("Connection", "close");
                exchange.sendResponseHeaders(302, -1);
            } catch (Exception e) {
                // Fail closed with safe redirect
                Headers headers = exchange.getResponseHeaders();
                headers.set("Location", "/");
                headers.set("Connection", "close");
                exchange.sendResponseHeaders(302, -1);
            } finally {
                try (OutputStream os = exchange.getResponseBody()) {
                    // no body
                }
            }
        }

        private String parseTarget(String rawQuery) {
            if (rawQuery == null) return null;
            if (rawQuery.length() > MAX_QUERY_LEN) return null;
            String[] parts = rawQuery.split("&");
            for (String part : parts) {
                int eq = part.indexOf('=');
                if (eq <= 0) continue;
                String name = part.substring(0, eq);
                if ("target".equals(name)) {
                    String rawVal = part.substring(eq + 1);
                    try {
                        String dec = URLDecoder.decode(rawVal, StandardCharsets.UTF_8.name());
                        if (dec.length() > MAX_PARAM_LEN) return null;
                        return dec;
                    } catch (IllegalArgumentException ex) {
                        return null;
                    }
                }
            }
            return null;
        }

        private void send(HttpExchange ex, int code, String body) throws IOException {
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            ex.getResponseHeaders().set("Connection", "close");
            ex.sendResponseHeaders(code, bytes.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(bytes);
            }
        }
    }
}