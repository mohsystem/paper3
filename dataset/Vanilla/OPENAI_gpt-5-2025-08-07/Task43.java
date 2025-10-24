import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.*;

public class Task43 {

    static class Session {
        final String token;
        final String user;
        volatile long expiresAt;

        Session(String token, String user, long expiresAt) {
            this.token = token;
            this.user = user;
            this.expiresAt = expiresAt;
        }
    }

    static class SessionManager {
        private final ConcurrentHashMap<String, Session> sessions = new ConcurrentHashMap<>();
        private final long ttlMillis;
        private final SecureRandom random = new SecureRandom();
        private final ScheduledExecutorService cleaner = Executors.newSingleThreadScheduledExecutor();

        SessionManager(long ttlMillis) {
            this.ttlMillis = ttlMillis;
            cleaner.scheduleAtFixedRate(this::cleanup, ttlMillis, ttlMillis, TimeUnit.MILLISECONDS);
        }

        String createSession(String user) {
            String token = generateToken(32);
            long exp = System.currentTimeMillis() + ttlMillis;
            Session s = new Session(token, user, exp);
            sessions.put(token, s);
            return token;
        }

        Session validate(String token) {
            if (token == null) return null;
            Session s = sessions.get(token);
            if (s == null) return null;
            if (System.currentTimeMillis() > s.expiresAt) {
                sessions.remove(token);
                return null;
            }
            return s;
        }

        boolean terminate(String token) {
            if (token == null) return false;
            return sessions.remove(token) != null;
        }

        private void cleanup() {
            long now = System.currentTimeMillis();
            for (Map.Entry<String, Session> e : sessions.entrySet()) {
                if (e.getValue().expiresAt < now) {
                    sessions.remove(e.getKey());
                }
            }
        }

        private String generateToken(int bytes) {
            byte[] buf = new byte[bytes];
            random.nextBytes(buf);
            StringBuilder sb = new StringBuilder(bytes * 2);
            for (byte b : buf) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        }
    }

    static class WebApp {
        private final SessionManager sessionManager;
        private final String cookieName = "SESSIONID";
        private final int cookieMaxAgeSeconds;

        WebApp(SessionManager sessionManager, int cookieMaxAgeSeconds) {
            this.sessionManager = sessionManager;
            this.cookieMaxAgeSeconds = cookieMaxAgeSeconds;
        }

        String login(String user) {
            return sessionManager.createSession(user);
        }

        Session me(String token) {
            return sessionManager.validate(token);
        }

        boolean logout(String token) {
            return sessionManager.terminate(token);
        }

        String getCookieFromHeaders(Headers headers) {
            List<String> cookieHeaders = headers.get("Cookie");
            if (cookieHeaders == null) return null;
            for (String header : cookieHeaders) {
                String[] parts = header.split(";");
                for (String p : parts) {
                    String[] kv = p.trim().split("=", 2);
                    if (kv.length == 2 && kv[0].equals(cookieName)) {
                        return kv[1];
                    }
                }
            }
            return null;
        }

        String makeSetCookie(String token, boolean expireNow) {
            if (expireNow) {
                return cookieName + "=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax";
            } else {
                return cookieName + "=" + token + "; Path=/; Max-Age=" + cookieMaxAgeSeconds + "; HttpOnly; SameSite=Lax";
            }
        }

        static Map<String, String> parseQuery(String query) {
            Map<String, String> map = new HashMap<>();
            if (query == null || query.isEmpty()) return map;
            String[] pairs = query.split("&");
            for (String pair : pairs) {
                String[] kv = pair.split("=", 2);
                String k = decode(kv[0]);
                String v = kv.length > 1 ? decode(kv[1]) : "";
                map.put(k, v);
            }
            return map;
        }

        private static String decode(String s) {
            return java.net.URLDecoder.decode(s, StandardCharsets.UTF_8);
        }
    }

    static class LoginHandler implements HttpHandler {
        private final WebApp app;

        LoginHandler(WebApp app) {
            this.app = app;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!exchange.getRequestMethod().equalsIgnoreCase("GET")) {
                send(exchange, 405, "Method Not Allowed");
                return;
            }
            URI uri = exchange.getRequestURI();
            Map<String, String> q = WebApp.parseQuery(uri.getRawQuery());
            String user = q.getOrDefault("user", "guest");
            String token = app.login(user);
            String body = "{\"status\":\"ok\",\"user\":\"" + user + "\"}";
            exchange.getResponseHeaders().add("Set-Cookie", app.makeSetCookie(token, false));
            exchange.getResponseHeaders().add("Content-Type", "application/json; charset=utf-8");
            send(exchange, 200, body);
        }
    }

    static class MeHandler implements HttpHandler {
        private final WebApp app;

        MeHandler(WebApp app) {
            this.app = app;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!exchange.getRequestMethod().equalsIgnoreCase("GET")) {
                send(exchange, 405, "Method Not Allowed");
                return;
            }
            String token = app.getCookieFromHeaders(exchange.getRequestHeaders());
            Session s = app.me(token);
            if (s == null) {
                send(exchange, 401, "{\"error\":\"unauthorized\"}");
                return;
            }
            String body = "{\"user\":\"" + s.user + "\",\"expiresAt\":\"" + Instant.ofEpochMilli(s.expiresAt).toString() + "\"}";
            exchange.getResponseHeaders().add("Content-Type", "application/json; charset=utf-8");
            send(exchange, 200, body);
        }
    }

    static class LogoutHandler implements HttpHandler {
        private final WebApp app;

        LogoutHandler(WebApp app) {
            this.app = app;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!exchange.getRequestMethod().equalsIgnoreCase("GET")) {
                send(exchange, 405, "Method Not Allowed");
                return;
            }
            String token = app.getCookieFromHeaders(exchange.getRequestHeaders());
            boolean ok = app.logout(token);
            exchange.getResponseHeaders().add("Set-Cookie", app.makeSetCookie("", true));
            if (ok) {
                send(exchange, 200, "{\"status\":\"logged_out\"}");
            } else {
                send(exchange, 200, "{\"status\":\"no_session\"}");
            }
        }
    }

    static void send(HttpExchange exchange, int status, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    public static void main(String[] args) throws Exception {
        int port = 8085;
        SessionManager sm = new SessionManager(TimeUnit.MINUTES.toMillis(15));
        WebApp app = new WebApp(sm, (int) TimeUnit.MINUTES.toSeconds(15));
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/login", new LoginHandler(app));
        server.createContext("/me", new MeHandler(app));
        server.createContext("/logout", new LogoutHandler(app));
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();

        // 5 test cases
        HttpClient client = HttpClient.newHttpClient();
        String base = "http://127.0.0.1:" + port;
        String sessionCookie = null;

        // Test 1: /me without cookie -> 401
        HttpRequest t1 = HttpRequest.newBuilder().uri(URI.create(base + "/me")).GET().build();
        HttpResponse<String> r1 = client.send(t1, HttpResponse.BodyHandlers.ofString());
        System.out.println("Test1 status: " + r1.statusCode() + " body: " + r1.body());

        // Test 2: /login?user=alice -> 200 and Set-Cookie
        HttpRequest t2 = HttpRequest.newBuilder().uri(URI.create(base + "/login?user=alice")).GET().build();
        HttpResponse<String> r2 = client.send(t2, HttpResponse.BodyHandlers.ofString());
        System.out.println("Test2 status: " + r2.statusCode() + " body: " + r2.body());
        Optional<String> setCookie = r2.headers().firstValue("Set-Cookie");
        if (setCookie.isPresent()) {
            String sc = setCookie.get();
            // Extract "SESSIONID=..."
            String[] parts = sc.split(";");
            for (String p : parts) {
                if (p.trim().startsWith("SESSIONID=")) {
                    sessionCookie = p.trim();
                    break;
                }
            }
        }

        // Test 3: /me with cookie -> 200
        HttpRequest.Builder tb3 = HttpRequest.newBuilder().uri(URI.create(base + "/me")).GET();
        if (sessionCookie != null) tb3.header("Cookie", sessionCookie);
        HttpResponse<String> r3 = client.send(tb3.build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test3 status: " + r3.statusCode() + " body: " + r3.body());

        // Test 4: /logout with cookie -> 200
        HttpRequest.Builder tb4 = HttpRequest.newBuilder().uri(URI.create(base + "/logout")).GET();
        if (sessionCookie != null) tb4.header("Cookie", sessionCookie);
        HttpResponse<String> r4 = client.send(tb4.build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test4 status: " + r4.statusCode() + " body: " + r4.body());

        // Test 5: /me with same cookie -> 401
        HttpRequest.Builder tb5 = HttpRequest.newBuilder().uri(URI.create(base + "/me")).GET();
        if (sessionCookie != null) tb5.header("Cookie", sessionCookie);
        HttpResponse<String> r5 = client.send(tb5.build(), HttpResponse.BodyHandlers.ofString());
        System.out.println("Test5 status: " + r5.statusCode() + " body: " + r5.body());

        // stop the server after tests
        server.stop(0);
        System.exit(0);
    }
}