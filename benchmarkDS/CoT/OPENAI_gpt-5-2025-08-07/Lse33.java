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
import java.util.HashMap;
import java.util.Map;

/*
Chain-of-Through in-code notes:
1) Problem understanding:
   - Provide a web route /go that accepts ?url=... 
   - redirect_invalid_url(url) validates domain is example.com, scheme is http/https, and no CRLF.
   - If valid: redirect (HTTP 302) to the URL; else: return error message (HTTP 400).

2) Security requirements:
   - Prevent open redirect by strictly validating scheme and host.
   - Disallow CRLF in user input to avoid header injection.
   - Use safe parsing via java.net.URI.

3) Secure coding generation:
   - Parse query safely.
   - Validate URI components.
   - Do not reflect untrusted input in headers unless validated.

4) Code review (embedded as comments and checks).

5) Secure code output:
   - Final handler uses validated URL or returns error.
*/

public class Lse33 {

    static class Decision {
        final boolean redirect;
        final String safeUrl; // when redirect == true
        final String error;   // when redirect == false
        Decision(boolean redirect, String safeUrl, String error) {
            this.redirect = redirect;
            this.safeUrl = safeUrl;
            this.error = error;
        }
    }

    // Core validation function: accepts URL as parameter, returns decision object.
    public static Decision redirect_invalid_url(String inputUrl) {
        if (inputUrl == null || inputUrl.isEmpty()) {
            return new Decision(false, null, "Missing URL");
        }
        // Basic CRLF/header injection guard
        if (inputUrl.indexOf('\r') >= 0 || inputUrl.indexOf('\n') >= 0) {
            return new Decision(false, null, "Invalid characters in URL");
        }
        try {
            URI uri = new URI(inputUrl.trim());
            String scheme = uri.getScheme();
            String host = uri.getHost();

            if (scheme == null || host == null) {
                return new Decision(false, null, "URL must include scheme and host");
            }
            String lowerScheme = scheme.toLowerCase();
            if (!("http".equals(lowerScheme) || "https".equals(lowerScheme))) {
                return new Decision(false, null, "Only http and https schemes are allowed");
            }
            if (!"example.com".equalsIgnoreCase(host)) {
                return new Decision(false, null, "URL must have domain example.com");
            }
            // Use an ASCII-safe representation for headers
            String safe = uri.toASCIIString();
            // Final sanity check after conversion
            if (safe.indexOf('\r') >= 0 || safe.indexOf('\n') >= 0) {
                return new Decision(false, null, "Invalid URL");
            }
            return new Decision(true, safe, null);
        } catch (Exception e) {
            return new Decision(false, null, "Invalid URL format");
        }
    }

    // Minimal HTTP server to expose route /go?url=...
    private static void startServer() throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/go", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed");
                    return;
                }
                Map<String, String> q = parseQuery(exchange.getRequestURI());
                String url = q.get("url");
                Decision d = redirect_invalid_url(url);
                if (d.redirect) {
                    Headers headers = exchange.getResponseHeaders();
                    headers.set("Location", d.safeUrl);
                    exchange.sendResponseHeaders(302, -1);
                    exchange.close();
                } else {
                    send(exchange, 400, d.error);
                }
            }
        });
        server.setExecutor(null);
        server.start();
        System.out.println("Java server running on http://localhost:8080/go?url=...");
    }

    private static void send(HttpExchange ex, int status, String body) throws IOException {
        byte[] data = body == null ? new byte[0] : body.getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        ex.sendResponseHeaders(status, data.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(data);
        }
    }

    private static Map<String, String> parseQuery(URI uri) {
        Map<String, String> map = new HashMap<>();
        String rawQuery = uri.getRawQuery();
        if (rawQuery == null || rawQuery.isEmpty()) return map;
        String[] pairs = rawQuery.split("&");
        for (String p : pairs) {
            int idx = p.indexOf('=');
            if (idx > 0) {
                String k = URLDecoder.decode(p.substring(0, idx), StandardCharsets.UTF_8);
                String v = URLDecoder.decode(p.substring(idx + 1), StandardCharsets.UTF_8);
                map.put(k, v);
            } else {
                String k = URLDecoder.decode(p, StandardCharsets.UTF_8);
                map.put(k, "");
            }
        }
        return map;
    }

    // Main with 5 test cases for redirect_invalid_url
    public static void main(String[] args) throws Exception {
        String[] tests = new String[]{
            "https://example.com/path",
            "http://example.com",
            "https://sub.example.com/path",  // invalid: subdomain
            "ftp://example.com/resource",    // invalid: scheme
            "https://evil.com/path"          // invalid: host
        };
        System.out.println("Java tests for redirect_invalid_url:");
        for (String t : tests) {
            Decision d = redirect_invalid_url(t);
            if (d.redirect) {
                System.out.println("OK redirect -> " + d.safeUrl);
            } else {
                System.out.println("ERROR -> " + d.error);
            }
        }
        // Start server
        startServer();
    }
}