import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.util.*;

/*
Chain-of-Through process (concise, non-sensitive):
1) Problem: Web app route /redirect/<url>, function redirect_invalid_url() allows redirects only to example.com, else error.
2) Security: Validate scheme, host equality with example.com, reject CR/LF, limit length.
3) Secure coding: Strict parse with URI, header sanitization, only http/https.
4) Review: Minimal dependencies, careful decoding, clear error handling.
5) Output: Final secure implementation with tests.
*/

public class Lse32 {

    static final int MAX_URL_LEN = 2048;

    public static class Result {
        public final int status;
        public final boolean redirect;
        public final String location;
        public final String body;

        public Result(int status, boolean redirect, String location, String body) {
            this.status = status;
            this.redirect = redirect;
            this.location = location;
            this.body = body;
        }
    }

    // URL percent-decoder for path segments (does not treat '+' as space)
    public static String percentDecode(String s) {
        if (s == null) return null;
        int len = s.length();
        byte[] out = new byte[len];
        int outLen = 0;
        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            if (c == '%' && i + 2 < len) {
                int hi = Character.digit(s.charAt(i + 1), 16);
                int lo = Character.digit(s.charAt(i + 2), 16);
                if (hi >= 0 && lo >= 0) {
                    out[outLen++] = (byte) ((hi << 4) + lo);
                    i += 2;
                } else {
                    out[outLen++] = (byte) c;
                }
            } else {
                out[outLen++] = (byte) c;
            }
        }
        return new String(out, 0, outLen, StandardCharsets.UTF_8);
    }

    private static boolean isSafeHost(String host) {
        return host != null && host.equalsIgnoreCase("example.com");
    }

    private static boolean hasCRLF(String s) {
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch == '\r' || ch == '\n') return true;
        }
        return false;
    }

    public static Result redirect_invalid_url(String inputUrl) {
        if (inputUrl == null) {
            return new Result(400, false, null, "Invalid or disallowed URL");
        }
        String url = inputUrl.trim();
        if (url.isEmpty() || url.length() > MAX_URL_LEN || hasCRLF(url)) {
            return new Result(400, false, null, "Invalid or disallowed URL");
        }
        try {
            URI uri = new URI(url);
            if (!uri.isAbsolute()) {
                return new Result(400, false, null, "Invalid or disallowed URL");
            }
            String scheme = Optional.ofNullable(uri.getScheme()).orElse("").toLowerCase(Locale.ROOT);
            if (!scheme.equals("http") && !scheme.equals("https")) {
                return new Result(400, false, null, "Invalid or disallowed URL");
            }
            String host = uri.getHost();
            if (!isSafeHost(host)) {
                return new Result(400, false, null, "Invalid or disallowed URL");
            }
            // Normalize URI to avoid oddities, rebuild without user-info fragment changes
            URI normalized = new URI(
                    scheme,
                    uri.getUserInfo(),
                    host,
                    uri.getPort(),
                    uri.getRawPath(),
                    uri.getRawQuery(),
                    uri.getRawFragment()
            );
            String location = normalized.toString();
            if (hasCRLF(location)) {
                return new Result(400, false, null, "Invalid or disallowed URL");
            }
            return new Result(302, true, location, "Redirecting...");
        } catch (Exception e) {
            return new Result(400, false, null, "Invalid or disallowed URL");
        }
    }

    // Simple HTTP handler for /redirect/<url>
    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                URI reqUri = exchange.getRequestURI();
                String rawPath = reqUri.getRawPath(); // keep percent-encoding
                if (rawPath == null) rawPath = "/";
                String prefix = "/redirect/";
                String urlParamEncoded = "";
                if (rawPath.equals("/redirect")) {
                    exchange.sendResponseHeaders(400, 0);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write("Invalid or disallowed URL".getBytes(StandardCharsets.UTF_8));
                    }
                    return;
                }
                if (rawPath.startsWith(prefix)) {
                    urlParamEncoded = rawPath.substring(prefix.length());
                } else {
                    exchange.sendResponseHeaders(404, 0);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write("Not Found".getBytes(StandardCharsets.UTF_8));
                    }
                    return;
                }
                String decodedUrl = percentDecode(urlParamEncoded);
                Result res = redirect_invalid_url(decodedUrl);
                Headers headers = exchange.getResponseHeaders();
                headers.set("Content-Type", "text/plain; charset=utf-8");
                if (res.redirect && res.location != null) {
                    headers.set("Location", res.location);
                }
                exchange.sendResponseHeaders(res.status, res.body == null ? -1 : res.body.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    if (res.body != null) os.write(res.body.getBytes(StandardCharsets.UTF_8));
                }
            } finally {
                exchange.close();
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 Test cases for redirect_invalid_url
        String[] tests = new String[] {
                "https://example.com/path?q=1",
                "http://example.com",
                "https://notexample.com/bad",
                "javascript:alert(1)",
                "https://example.com:443/ok#frag"
        };
        for (String t : tests) {
            Result r = redirect_invalid_url(t);
            System.out.println("Test: " + t + " => status=" + r.status + ", redirect=" + r.redirect + (r.location != null ? (", location=" + r.location) : ""));
        }

        // Start HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/redirect", new RedirectHandler());
        server.setExecutor(null);
        System.out.println("Java server listening on http://127.0.0.1:8080");
        server.start();
    }
}