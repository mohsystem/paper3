import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.*;

public class Task38 {
    private static final int PORT = 8080;
    private static final int MAX_BODY_BYTES = 4096;
    private static final int MAX_INPUT_LEN = 500;
    private static final String CSRF_COOKIE = "csrf";
    private static final SecureRandom RNG = new SecureRandom();

    // Entry point with 5 test cases and server startup
    public static void main(String[] args) throws Exception {
        // 5 test cases for processInput
        String[] tests = new String[] {
                "<script>alert('x')</script>",
                "Hello, World!",
                "Quotes: \" ' & < >",
                "Unicode: Привет & こんにちは <b>bold</b>",
                "A".repeat(600)
        };
        for (int i = 0; i < tests.length; i++) {
            String out = processInput(tests[i]);
            System.out.println("Test " + (i + 1) + " -> " + out);
        }

        InetSocketAddress addr = new InetSocketAddress("127.0.0.1", PORT);
        HttpServer server = HttpServer.create(addr, 0);
        server.createContext("/", new RootHandler());
        server.createContext("/submit", new SubmitHandler());
        server.setExecutor(null);
        server.start();
        // Server is running. Visit http://127.0.0.1:8080/ in a browser.
    }

    // Process user input securely: trim, limit length, and escape HTML
    public static String processInput(String input) {
        if (input == null) return "";
        String trimmed = input.strip();
        if (trimmed.length() > MAX_INPUT_LEN) {
            trimmed = trimmed.substring(0, MAX_INPUT_LEN);
        }
        return escapeHtml(trimmed);
    }

    // HTML escaping to mitigate XSS
    public static String escapeHtml(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(s.length());
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    // Generate a CSRF token (Base64 URL-safe without padding)
    public static String generateToken() {
        byte[] bytes = new byte[32];
        RNG.nextBytes(bytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    // Read request body with size limit
    public static String readRequestBody(HttpExchange ex, int maxBytes) throws IOException {
        try (InputStream is = ex.getRequestBody();
             ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
            byte[] buf = new byte[1024];
            int total = 0;
            int r;
            while ((r = is.read(buf)) != -1) {
                total += r;
                if (total > maxBytes) {
                    throw new IOException("Body too large");
                }
                baos.write(buf, 0, r);
            }
            return baos.toString(StandardCharsets.UTF_8);
        }
    }

    // Parse application/x-www-form-urlencoded body
    public static Map<String, String> parseForm(String body) {
        Map<String, String> map = new HashMap<>();
        if (body == null || body.isEmpty()) return map;
        String[] pairs = body.split("&");
        for (String pair : pairs) {
            String[] kv = pair.split("=", 2);
            String key = urlDecode(kv[0]);
            String val = kv.length > 1 ? urlDecode(kv[1]) : "";
            if (!key.isEmpty()) {
                map.put(key, val);
            }
        }
        return map;
    }

    public static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return "";
        }
    }

    // Parse cookies into a map
    public static Map<String, String> parseCookies(Headers headers) {
        Map<String, String> cookies = new HashMap<>();
        List<String> cookieHeaders = headers.get("Cookie");
        if (cookieHeaders == null) return cookies;
        for (String header : cookieHeaders) {
            String[] parts = header.split(";");
            for (String part : parts) {
                String[] kv = part.trim().split("=", 2);
                if (kv.length == 2) {
                    cookies.put(kv[0].trim(), kv[1].trim());
                }
            }
        }
        return cookies;
    }

    // Set cookie with secure attributes
    public static void setCookie(HttpExchange ex, String name, String value, String path, boolean httpOnly, boolean secure, String sameSite) {
        StringBuilder sb = new StringBuilder();
        sb.append(name).append("=").append(value).append("; Path=").append(path == null ? "/" : path);
        if (httpOnly) sb.append("; HttpOnly");
        if (secure) sb.append("; Secure");
        if (sameSite != null && !sameSite.isEmpty()) {
            sb.append("; SameSite=").append(sameSite);
        }
        ex.getResponseHeaders().add("Set-Cookie", sb.toString());
    }

    // Add security headers
    public static void addSecurityHeaders(HttpExchange ex) {
        Headers h = ex.getResponseHeaders();
        h.set("Content-Security-Policy", "default-src 'none'; style-src 'self' 'unsafe-inline'; form-action 'self'");
        h.set("X-Content-Type-Options", "nosniff");
        h.set("X-Frame-Options", "DENY");
        h.set("Referrer-Policy", "no-referrer");
        h.set("Content-Type", "text/html; charset=UTF-8");
        h.set("Connection", "close");
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(ex.getRequestMethod())) {
                    sendPlain(ex, 405, "Method Not Allowed");
                    return;
                }
                addSecurityHeaders(ex);

                Map<String, String> cookies = parseCookies(ex.getRequestHeaders());
                String csrf = cookies.get(CSRF_COOKIE);
                if (csrf == null || csrf.isEmpty()) {
                    csrf = generateToken();
                    setCookie(ex, CSRF_COOKIE, csrf, "/", true, true, "Strict");
                }

                String body = "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">" +
                        "<title>Echo</title></head><body>" +
                        "<h1>Secure Echo</h1>" +
                        "<form method=\"post\" action=\"/submit\">" +
                        "<label for=\"msg\">Enter text:</label> " +
                        "<input id=\"msg\" name=\"message\" type=\"text\" maxlength=\"" + MAX_INPUT_LEN + "\" required>" +
                        "<input type=\"hidden\" name=\"csrf\" value=\"" + escapeHtml(csrf) + "\">" +
                        "<button type=\"submit\">Submit</button>" +
                        "</form></body></html>";
                byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
                ex.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = ex.getResponseBody()) {
                    os.write(bytes);
                }
            } catch (Exception e) {
                safeServerError(ex);
            } finally {
                ex.close();
            }
        }
    }

    static class SubmitHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(ex.getRequestMethod())) {
                    sendPlain(ex, 405, "Method Not Allowed");
                    return;
                }
                String ct = ex.getRequestHeaders().getFirst("Content-Type");
                if (ct == null || !ct.toLowerCase(Locale.ROOT).startsWith("application/x-www-form-urlencoded")) {
                    sendPlain(ex, 415, "Unsupported Media Type");
                    return;
                }

                String rawBody;
                try {
                    rawBody = readRequestBody(ex, MAX_BODY_BYTES);
                } catch (IOException tooBig) {
                    sendPlain(ex, 413, "Payload Too Large");
                    return;
                }
                Map<String, String> form = parseForm(rawBody);
                String message = form.getOrDefault("message", "");
                String csrfForm = form.getOrDefault("csrf", "");

                Map<String, String> cookies = parseCookies(ex.getRequestHeaders());
                String csrfCookie = cookies.get(CSRF_COOKIE);
                if (csrfCookie == null || !csrfCookie.equals(csrfForm)) {
                    sendPlain(ex, 400, "Bad Request");
                    return;
                }

                String safe = processInput(message);

                addSecurityHeaders(ex);
                String body = "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">" +
                        "<title>Echo Result</title></head><body>" +
                        "<h1>Echoed back safely:</h1>" +
                        "<p>" + safe + "</p>" +
                        "<p><a href=\"/\">Back</a></p>" +
                        "</body></html>";
                byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
                ex.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = ex.getResponseBody()) {
                    os.write(bytes);
                }
            } catch (Exception e) {
                safeServerError(ex);
            } finally {
                ex.close();
            }
        }
    }

    private static void safeServerError(HttpExchange ex) throws IOException {
        try {
            sendPlain(ex, 500, "Internal Server Error");
        } catch (Exception ignore) {
        }
    }

    private static void sendPlain(HttpExchange ex, int code, String msg) throws IOException {
        addSecurityHeaders(ex);
        byte[] b = msg.getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
        ex.sendResponseHeaders(code, b.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(b);
        }
    }
}