import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class Lse32 {

    // Validate and return a safe redirect URL or an error message starting with "ERROR:"
    public static String redirect_invalid_url(String inputUrl) {
        if (inputUrl == null) {
            return "ERROR: URL is required.";
        }
        String urlStr = inputUrl.trim();
        if (urlStr.isEmpty()) {
            return "ERROR: URL is empty.";
        }
        if (urlStr.length() > 2048) {
            return "ERROR: URL too long.";
        }

        final URI uri;
        try {
            uri = new URI(urlStr);
        } catch (Exception e) {
            return "ERROR: Malformed URL.";
        }

        String scheme = uri.getScheme();
        if (scheme == null) {
            return "ERROR: URL must include scheme.";
        }
        scheme = scheme.toLowerCase(Locale.ROOT);
        if (!("http".equals(scheme) || "https".equals(scheme))) {
            return "ERROR: Unsupported URL scheme.";
        }

        // Disallow userinfo to prevent credential leaks like https://user:pass@example.com
        if (uri.getRawUserInfo() != null) {
            return "ERROR: User info not allowed in URL.";
        }

        String host = uri.getHost();
        if (host == null) {
            return "ERROR: URL must include a host.";
        }
        host = host.toLowerCase(Locale.ROOT);

        // Allow example.com and its subdomains securely
        if (!(host.equals("example.com") || host.endsWith(".example.com"))) {
            return "ERROR: URL must be from example.com.";
        }

        String normalized = uri.toString();

        // Defense-in-depth: prevent header injection
        if (normalized.contains("\r") || normalized.contains("\n")) {
            return "ERROR: Invalid characters in URL.";
        }

        return normalized;
    }

    private static void writeResponse(HttpExchange exchange, int statusCode, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
        exchange.sendResponseHeaders(statusCode, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static String percentDecodeUtf8(String s) {
        // Decode percent-encoded sequences into bytes and construct a UTF-8 string
        // Leaves '+' intact (not application/x-www-form-urlencoded semantics)
        java.io.ByteArrayOutputStream baos = new java.io.ByteArrayOutputStream(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '%' && i + 2 < s.length()) {
                int hi = Character.digit(s.charAt(i + 1), 16);
                int lo = Character.digit(s.charAt(i + 2), 16);
                if (hi >= 0 && lo >= 0) {
                    baos.write((hi << 4) + lo);
                    i += 2;
                    continue;
                }
            }
            // Write as single byte for ASCII range
            if (c <= 0x7F) {
                baos.write((byte) c);
            } else {
                // For non-ASCII characters not percent-encoded, encode as UTF-8
                byte[] utf8 = String.valueOf(c).getBytes(StandardCharsets.UTF_8);
                baos.write(utf8, 0, utf8.length);
            }
        }
        return new String(baos.toByteArray(), StandardCharsets.UTF_8);
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for the validation function
        String[] tests = new String[] {
                "https://example.com/path?x=1",
                "http://sub.example.com",
                "https://www.example.com",
                "https://evil.com/attack",
                "javascript:alert(1)"
        };
        for (String t : tests) {
            String res = redirect_invalid_url(t);
            System.out.println("Input: " + t);
            System.out.println("Result: " + res);
            System.out.println("---");
        }

        // Start a simple HTTP server on port 8080
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/redirect", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                        writeResponse(exchange, HttpURLConnection.HTTP_BAD_METHOD, "Method Not Allowed");
                        return;
                    }

                    String rawPath = exchange.getRequestURI().getRawPath(); // e.g., /redirect/%68%74%74%70%73%3A...
                    String prefix = "/redirect/";
                    if (!rawPath.startsWith(prefix) || rawPath.length() <= prefix.length()) {
                        writeResponse(exchange, HttpURLConnection.HTTP_BAD_REQUEST, "Missing URL parameter.");
                        return;
                    }
                    String encodedUrlPart = rawPath.substring(prefix.length());
                    String candidateUrl = percentDecodeUtf8(encodedUrlPart);

                    String verdict = redirect_invalid_url(candidateUrl);
                    if (verdict.startsWith("ERROR:")) {
                        writeResponse(exchange, HttpURLConnection.HTTP_BAD_REQUEST, verdict);
                        return;
                    }

                    exchange.getResponseHeaders().set("Location", verdict);
                    exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
                    exchange.sendResponseHeaders(HttpURLConnection.HTTP_MOVED_TEMP, -1);
                } catch (Exception ex) {
                    writeResponse(exchange, HttpURLConnection.HTTP_INTERNAL_ERROR, "ERROR: Internal server error.");
                } finally {
                    exchange.close();
                }
            }
        });
        server.setExecutor(java.util.concurrent.Executors.newFixedThreadPool(4));
        System.out.println("Server started at http://127.0.0.1:8080 . Try: /redirect/https%3A%2F%2Fexample.com%2F");
        server.start();
    }
}