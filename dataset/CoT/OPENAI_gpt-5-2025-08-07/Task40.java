import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.*;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.*;

public class Task40 {
    // Chain-of-Through steps (in comments):
    // 1) Problem understanding: Build a minimal web app exposing a settings form (GET) and update endpoint (POST),
    //    secured against CSRF using per-session tokens stored server-side and validated on POST.
    // 2) Security requirements: Session management with secure random IDs, HttpOnly cookie, SameSite=Strict,
    //    CSRF token bound to session, constant-time compare, input parsing, HTML escaping, safe headers.
    // 3) Secure coding generation: Implement server, endpoints, session store, CSRF logic, and tests.
    // 4) Code review: Inline via careful parsing, error checks, header settings, token rotation, one-time-use token.
    // 5) Secure code output: Final code below.

    public static void main(String[] args) throws Exception {
        int port = 8088;
        SecureHttpServer server = new SecureHttpServer(port);
        server.start();

        // 5 test cases using built-in HttpClient
        Thread.sleep(300); // small delay to ensure server is up

        System.out.println("Running Java tests against http://localhost:" + port);

        // Shared HTTP client
        HttpClient client = HttpClient.newBuilder()
                .connectTimeout(Duration.ofSeconds(3))
                .followRedirects(HttpClient.Redirect.NEVER)
                .build();

        // Helpers
        class CookieJar {
            Map<String, String> cookies = new HashMap<>();
            String toHeader() {
                if (cookies.isEmpty()) return "";
                StringBuilder sb = new StringBuilder();
                boolean first = true;
                for (Map.Entry<String, String> e : cookies.entrySet()) {
                    if (!first) sb.append("; ");
                    sb.append(e.getKey()).append("=").append(e.getValue());
                    first = false;
                }
                return sb.toString();
            }
            void storeFromHeaders(HttpHeaders headers) {
                List<String> setCookies = headers.map().getOrDefault("Set-Cookie", List.of());
                for (String sc : setCookies) {
                    String[] parts = sc.split(";", 2);
                    String[] kv = parts[0].split("=", 2);
                    if (kv.length == 2) {
                        cookies.put(kv[0].trim(), kv[1].trim());
                    }
                }
            }
        }
        CookieJar jar1 = new CookieJar();

        // Test 1: GET form -> obtain CSRF and cookie; POST with correct token should succeed (200)
        HttpResponse<String> resp1 = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .GET().build(), HttpResponse.BodyHandlers.ofString());
        jar1.storeFromHeaders(resp1.headers());
        String token1 = extractHiddenValue(resp1.body(), "csrf_token");
        HttpResponse<String> resp1b = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .header("Cookie", jar1.toHeader())
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(HttpRequest.BodyPublishers.ofString(urlEncode(Map.of(
                        "csrf_token", token1,
                        "displayName", "Alice",
                        "email", "alice@example.com"
                )))).build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test1 status=" + resp1b.statusCode()); // expect 200

        // Test 2: POST with missing token -> 403
        HttpResponse<String> resp2 = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .header("Cookie", jar1.toHeader())
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(HttpRequest.BodyPublishers.ofString(urlEncode(Map.of(
                        "displayName", "Bob",
                        "email", "bob@example.com"
                )))).build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test2 status=" + resp2.statusCode()); // expect 403

        // Test 3: POST with invalid token -> 403
        HttpResponse<String> resp3 = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .header("Cookie", jar1.toHeader())
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(HttpRequest.BodyPublishers.ofString(urlEncode(Map.of(
                        "csrf_token", "invalid-token",
                        "displayName", "Mallory",
                        "email", "mallory@example.com"
                )))).build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test3 status=" + resp3.statusCode()); // expect 403

        // Test 4: Use valid token but with wrong/absent session cookie -> 403
        // Acquire a fresh valid token with jar2
        CookieJar jar2 = new CookieJar();
        HttpResponse<String> resp4a = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .GET().build(), HttpResponse.BodyHandlers.ofString());
        jar2.storeFromHeaders(resp4a.headers());
        String token2 = extractHiddenValue(resp4a.body(), "csrf_token");
        // Now try POST with token2 but no cookie (wrong session)
        HttpResponse<String> resp4 = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(HttpRequest.BodyPublishers.ofString(urlEncode(Map.of(
                        "csrf_token", token2,
                        "displayName", "Eve",
                        "email", "eve@example.com"
                )))).build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test4 status=" + resp4.statusCode()); // expect 403

        // Test 5: Token one-time-use check: reuse previously valid token should fail after successful POST
        // Get new token3
        CookieJar jar3 = new CookieJar();
        HttpResponse<String> resp5a = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .GET().build(), HttpResponse.BodyHandlers.ofString());
        jar3.storeFromHeaders(resp5a.headers());
        String token3 = extractHiddenValue(resp5a.body(), "csrf_token");
        // First POST succeed
        HttpResponse<String> resp5b = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .header("Cookie", jar3.toHeader())
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(HttpRequest.BodyPublishers.ofString(urlEncode(Map.of(
                        "csrf_token", token3,
                        "displayName", "Carol",
                        "email", "carol@example.com"
                )))).build(), HttpResponse.BodyHandlers.ofString());
        // Reuse same token should fail
        HttpResponse<String> resp5c = client.send(HttpRequest.newBuilder()
                .uri(URI.create("http://localhost:" + port + "/settings"))
                .header("Cookie", jar3.toHeader())
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(HttpRequest.BodyPublishers.ofString(urlEncode(Map.of(
                        "csrf_token", token3,
                        "displayName", "Carol2",
                        "email", "carol2@example.com"
                )))).build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test5 first status=" + resp5b.statusCode() + " second status=" + resp5c.statusCode()); // expect 200 then 403

        // Stop server after tests
        server.stop();
    }

    // Utility to extract hidden input value by name
    private static String extractHiddenValue(String html, String name) {
        String needle = "name=\"" + name + "\"";
        int idx = html.indexOf(needle);
        if (idx < 0) return null;
        int valIdx = html.indexOf("value=\"", idx);
        if (valIdx < 0) return null;
        valIdx += 7;
        int end = html.indexOf("\"", valIdx);
        if (end < 0) return null;
        return html.substring(valIdx, end);
    }

    private static String urlEncode(Map<String, String> params) {
        StringBuilder sb = new StringBuilder();
        try {
            boolean first = true;
            for (Map.Entry<String, String> e : params.entrySet()) {
                if (!first) sb.append("&");
                sb.append(URLEncoder.encode(e.getKey(), "UTF-8"))
                        .append("=")
                        .append(URLEncoder.encode(e.getValue(), "UTF-8"));
                first = false;
            }
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException(ex);
        }
        return sb.toString();
    }

    // Secure server implementation
    static class SecureHttpServer {
        private final HttpServer server;
        private final SessionStore sessions = new SessionStore();

        SecureHttpServer(int port) throws IOException {
            server = HttpServer.create(new InetSocketAddress("localhost", port), 0);
            server.createContext("/", new RootHandler());
            server.createContext("/settings", new SettingsHandler(sessions));
            server.setExecutor(Executors.newCachedThreadPool());
        }

        void start() {
            server.start();
        }
        void stop() {
            server.stop(0);
        }

        static class RootHandler implements HttpHandler {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                Headers h = exchange.getResponseHeaders();
                setSecurityHeaders(h);
                h.add("Location", "/settings");
                exchange.sendResponseHeaders(302, -1);
                exchange.close();
            }
        }

        static class SettingsHandler implements HttpHandler {
            private final SessionStore sessions;
            private static final SecureRandom RNG = new SecureRandom();

            SettingsHandler(SessionStore sessions) {
                this.sessions = sessions;
            }

            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    String method = exchange.getRequestMethod();
                    if ("GET".equalsIgnoreCase(method)) {
                        handleGet(exchange);
                    } else if ("POST".equalsIgnoreCase(method)) {
                        handlePost(exchange);
                    } else {
                        sendText(exchange, 405, "Method Not Allowed");
                    }
                } finally {
                    exchange.close();
                }
            }

            private void handleGet(HttpExchange ex) throws IOException {
                Headers h = ex.getResponseHeaders();
                setSecurityHeaders(h);

                // Session management
                Map<String, String> cookies = parseCookies(ex.getRequestHeaders().getFirst("Cookie"));
                String sid = cookies.get("SESSIONID");
                Session session = sid != null ? sessions.get(sid) : null;
                if (session == null) {
                    sid = generateId(32);
                    session = new Session();
                    sessions.put(sid, session);
                }
                // Ensure CSRF token present
                if (session.csrfToken == null || session.csrfUsed) {
                    session.csrfToken = generateId(32);
                    session.csrfUsed = false;
                }

                // Set cookie
                h.add("Set-Cookie", "SESSIONID=" + sid + "; HttpOnly; Path=/; SameSite=Strict");

                String displayName = session.settings.getOrDefault("displayName", "");
                String email = session.settings.getOrDefault("email", "");
                String body = "<!doctype html><html><head><meta charset=\"utf-8\">" +
                        "<title>User Settings</title></head><body>" +
                        "<h1>User Settings</h1>" +
                        "<form method=\"POST\" action=\"/settings\">" +
                        "<input type=\"hidden\" name=\"csrf_token\" value=\"" + escapeHtml(session.csrfToken) + "\"/>" +
                        "<label>Display Name: <input name=\"displayName\" value=\"" + escapeHtml(displayName) + "\"/></label><br/>" +
                        "<label>Email: <input name=\"email\" value=\"" + escapeHtml(email) + "\"/></label><br/>" +
                        "<button type=\"submit\">Save</button>" +
                        "</form>" +
                        "</body></html>";

                h.set("Content-Type", "text/html; charset=utf-8");
                byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
                ex.sendResponseHeaders(200, bytes.length);
                ex.getResponseBody().write(bytes);
            }

            private void handlePost(HttpExchange ex) throws IOException {
                Headers h = ex.getResponseHeaders();
                setSecurityHeaders(h);

                String ctype = Optional.ofNullable(ex.getRequestHeaders().getFirst("Content-Type")).orElse("");
                if (!ctype.toLowerCase(Locale.ROOT).startsWith("application/x-www-form-urlencoded")) {
                    sendText(ex, 415, "Unsupported Media Type");
                    return;
                }

                Map<String, String> cookies = parseCookies(ex.getRequestHeaders().getFirst("Cookie"));
                String sid = cookies.get("SESSIONID");
                if (sid == null) {
                    sendText(ex, 403, "Forbidden: no session");
                    return;
                }
                Session session = sessions.get(sid);
                if (session == null) {
                    sendText(ex, 403, "Forbidden: invalid session");
                    return;
                }

                String body = readBody(ex.getRequestBody());
                Map<String, String> form = parseUrlEncoded(body);

                String token = form.get("csrf_token");
                if (token == null || session.csrfToken == null) {
                    sendText(ex, 403, "Forbidden: missing CSRF token");
                    return;
                }
                if (session.csrfUsed || !constantTimeEquals(token, session.csrfToken)) {
                    sendText(ex, 403, "Forbidden: invalid CSRF token");
                    return;
                }

                // Update settings safely
                String displayName = form.getOrDefault("displayName", "");
                String email = form.getOrDefault("email", "");
                // Basic length limits to prevent abuse
                if (displayName.length() > 100 || email.length() > 254) {
                    sendText(ex, 400, "Bad Request: input too long");
                    return;
                }
                session.settings.put("displayName", displayName);
                session.settings.put("email", email);

                // Rotate CSRF token (one-time use)
                session.csrfUsed = true;
                session.csrfToken = generateId(32);
                session.csrfUsed = false; // enable new token

                h.set("Content-Type", "text/plain; charset=utf-8");
                byte[] out = "Settings updated securely.".getBytes(StandardCharsets.UTF_8);
                ex.sendResponseHeaders(200, out.length);
                ex.getResponseBody().write(out);
            }

            private static String generateId(int bytesLen) {
                byte[] b = new byte[bytesLen];
                RNG.nextBytes(b);
                return Base64.getUrlEncoder().withoutPadding().encodeToString(b);
            }

            private static String readBody(InputStream is) throws IOException {
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[4096];
                int r;
                while ((r = is.read(buf)) != -1) bos.write(buf, 0, r);
                return bos.toString(StandardCharsets.UTF_8);
            }
        }

        static void setSecurityHeaders(Headers h) {
            h.set("X-Content-Type-Options", "nosniff");
            h.set("X-Frame-Options", "DENY");
            h.set("Referrer-Policy", "no-referrer");
            h.set("Content-Security-Policy", "default-src 'none'; style-src 'self' 'unsafe-inline'; img-src 'self'; form-action 'self'; base-uri 'none'");
        }
    }

    static class Session {
        String csrfToken;
        boolean csrfUsed;
        Map<String, String> settings = new ConcurrentHashMap<>();
    }

    static class SessionStore {
        private final ConcurrentHashMap<String, Session> map = new ConcurrentHashMap<>();
        Session get(String id) { return map.get(id); }
        void put(String id, Session s) { map.put(id, s); }
    }

    static Map<String, String> parseCookies(String cookieHeader) {
        Map<String, String> map = new HashMap<>();
        if (cookieHeader == null || cookieHeader.isEmpty()) return map;
        String[] parts = cookieHeader.split(";");
        for (String p : parts) {
            String[] kv = p.trim().split("=", 2);
            if (kv.length == 2) {
                map.put(kv[0].trim(), kv[1].trim());
            }
        }
        return map;
    }

    static Map<String, String> parseUrlEncoded(String body) {
        Map<String, String> map = new HashMap<>();
        if (body == null || body.isEmpty()) return map;
        String[] parts = body.split("&");
        for (String p : parts) {
            String[] kv = p.split("=", 2);
            try {
                String k = URLDecoder.decode(kv[0], "UTF-8");
                String v = kv.length > 1 ? URLDecoder.decode(kv[1], "UTF-8") : "";
                map.put(k, v);
            } catch (UnsupportedEncodingException e) {
                // ignore
            }
        }
        return map;
    }

    static String escapeHtml(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(Math.max(16, s.length()));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&' -> out.append("&amp;");
                case '<' -> out.append("&lt;");
                case '>' -> out.append("&gt;");
                case '"' -> out.append("&quot;");
                case '\'' -> out.append("&#x27;");
                case '/' -> out.append("&#x2F;");
                default -> out.append(c);
            }
        }
        return out.toString();
    }

    static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] ha = md.digest(a.getBytes(StandardCharsets.UTF_8));
            byte[] hb = md.digest(b.getBytes(StandardCharsets.UTF_8));
            if (ha.length != hb.length) return false;
            int res = 0;
            for (int i = 0; i < ha.length; i++) res |= ha[i] ^ hb[i];
            return res == 0;
        } catch (Exception e) {
            return a.equals(b);
        }
    }
}