import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;

public class Lse42 {
    // In-memory session store: SID -> {username, email, ssn}
    private static final Map<String, Map<String, String>> SESSIONS = new ConcurrentHashMap<>();

    // Info function: returns "username=...; email=...; ssn=..."
    public static String info(Map<String, String> session) {
        Objects.requireNonNull(session, "session");
        String username = session.get("username");
        String email = session.get("email");
        String ssn = session.get("ssn");
        if (username == null || email == null || ssn == null) {
            return null;
        }
        return "username=" + username + "; email=" + email + "; ssn=" + ssn;
    }

    // Minimal cookie parser to get SID
    private static String extractSid(String cookieHeader) {
        if (cookieHeader == null || cookieHeader.isEmpty()) return null;
        String[] parts = cookieHeader.split(";");
        for (String p : parts) {
            String s = p.trim();
            int eq = s.indexOf('=');
            if (eq > 0) {
                String k = s.substring(0, eq).trim();
                String v = s.substring(eq + 1).trim();
                if ("SID".equalsIgnoreCase(k)) {
                    if (v.length() >= 8 && v.length() <= 128) {
                        for (int i = 0; i < v.length(); i++) {
                            char c = v.charAt(i);
                            if (!Character.isLetterOrDigit(c)) {
                                return null;
                            }
                        }
                        return v;
                    }
                    return null;
                }
            }
        }
        return null;
    }

    // Handler for /info
    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"/info".equals(exchange.getRequestURI().getPath())) {
                    send(exchange, 404, "Not Found");
                    return;
                }
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed");
                    return;
                }

                Headers headers = exchange.getRequestHeaders();
                String cookie = headers.getFirst("Cookie");
                String sid = extractSid(cookie);
                if (sid == null) {
                    send(exchange, 401, "Unauthorized");
                    return;
                }
                Map<String, String> session = SESSIONS.get(sid);
                if (session == null) {
                    send(exchange, 401, "Unauthorized");
                    return;
                }

                String body = info(session);
                if (body == null) {
                    send(exchange, 400, "Bad Request");
                    return;
                }

                send(exchange, 200, body);
            } finally {
                exchange.close();
            }
        }

        private void send(HttpExchange ex, int code, String body) throws IOException {
            byte[] data = body.getBytes(StandardCharsets.UTF_8);
            Headers resp = ex.getResponseHeaders();
            resp.set("Content-Type", "text/plain; charset=UTF-8");
            resp.set("Cache-Control", "no-store");
            ex.sendResponseHeaders(code, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }
    }

    private static boolean httpTest(String method, String url, String cookie, int expectedStatus, String expectedBodyOrNull) {
        HttpURLConnection conn = null;
        try {
            URL u = new URL(url);
            conn = (HttpURLConnection) u.openConnection();
            conn.setRequestMethod(method);
            conn.setUseCaches(false);
            conn.setInstanceFollowRedirects(false);
            conn.setConnectTimeout(2000);
            conn.setReadTimeout(2000);
            if (cookie != null) {
                conn.setRequestProperty("Cookie", cookie);
            }
            int code = conn.getResponseCode();
            boolean statusOk = (code == expectedStatus);
            boolean bodyOk = true;
            if (expectedBodyOrNull != null && code == 200) {
                byte[] buf = conn.getInputStream().readAllBytes();
                String body = new String(buf, StandardCharsets.UTF_8);
                bodyOk = expectedBodyOrNull.equals(body);
            }
            return statusOk && bodyOk;
        } catch (Exception e) {
            return false;
        } finally {
            if (conn != null) {
                conn.disconnect();
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // Pre-populate sessions (test data only)
        String SID1 = "b7f1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1";
        String SID2 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        String SID3 = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

        Map<String, String> s1 = new HashMap<>();
        s1.put("username", "alice");
        s1.put("email", "alice@example.com");
        s1.put("ssn", "123-45-6789");
        SESSIONS.put(SID1, Collections.unmodifiableMap(s1));

        Map<String, String> s2 = new HashMap<>();
        s2.put("username", "bob");
        s2.put("email", "bob@example.com");
        // missing ssn to test 400
        SESSIONS.put(SID2, Collections.unmodifiableMap(s2));

        Map<String, String> s3 = new HashMap<>();
        s3.put("username", "carol");
        s3.put("email", "carol@example.com");
        s3.put("ssn", "987-65-4321");
        SESSIONS.put(SID3, Collections.unmodifiableMap(s3));

        int port = 18085;
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();

        // Allow server to start
        Thread.sleep(200);

        String base = "http://127.0.0.1:" + port + "/info";
        int passed = 0, total = 5;

        // Test 1: valid SID should return 200 with expected body (but do not print sensitive body)
        String expected1 = "username=alice; email=alice@example.com; ssn=123-45-6789";
        if (httpTest("GET", base, "SID=" + SID1, 200, expected1)) passed++;

        // Test 2: missing cookie -> 401
        if (httpTest("GET", base, null, 401, null)) passed++;

        // Test 3: unknown SID -> 401
        if (httpTest("GET", base, "SID=" + "deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef", 401, null)) passed++;

        // Test 4: session missing ssn -> 400
        if (httpTest("GET", base, "SID=" + SID2, 400, null)) passed++;

        // Test 5: wrong method -> 405
        if (httpTest("POST", base, "SID=" + SID1, 405, null)) passed++;

        System.out.println(String.format(Locale.ROOT, "Tests passed: %d/%d", passed, total));
        server.stop(0);
    }
}