import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.*;

public class Task43 {

    // Simple in-memory user store with PBKDF2 password hashing
    static class UserStore {
        static class User {
            String username;
            byte[] salt;
            byte[] hash;
            int iterations;
        }

        private final Map<String, User> users = new ConcurrentHashMap<>();
        private final SecureRandom rng = new SecureRandom();
        private final int defaultIterations = 150_000;
        private final int saltLen = 16;
        private final int dkLen = 32; // 256-bit

        public void addUser(String username, String password) {
            if (username == null || password == null) throw new IllegalArgumentException("Nulls not allowed");
            byte[] salt = new byte[saltLen];
            rng.nextBytes(salt);
            byte[] hash = pbkdf2(password.toCharArray(), salt, defaultIterations, dkLen);
            User u = new User();
            u.username = username;
            u.salt = salt;
            u.hash = hash;
            u.iterations = defaultIterations;
            users.put(username, u);
        }

        public boolean verify(String username, String password) {
            if (username == null || password == null) return false;
            User u = users.get(username);
            if (u == null) {
                // simulate hash time to prevent user enumeration timing
                byte[] fakeSalt = new byte[saltLen];
                rng.nextBytes(fakeSalt);
                byte[] fakeHash = pbkdf2(password.toCharArray(), fakeSalt, defaultIterations, dkLen);
                return false;
            }
            byte[] calc = pbkdf2(password.toCharArray(), u.salt, u.iterations, u.hash.length);
            return MessageDigest.isEqual(u.hash, calc);
        }

        private byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) {
            try {
                PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                return skf.generateSecret(spec).getEncoded();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }

    // In-memory session manager with secure random IDs and expiry
    static class SessionManager {
        static class Session {
            String user;
            long expiresAt;
        }

        private final ConcurrentHashMap<String, Session> sessions = new ConcurrentHashMap<>();
        private final SecureRandom rng = new SecureRandom();
        private final long sessionTtlMillis = 30 * 60 * 1000L; // 30m

        public String createSession(String user) {
            byte[] rnd = new byte[32];
            rng.nextBytes(rnd);
            String id = Base64.getUrlEncoder().withoutPadding().encodeToString(rnd);
            Session s = new Session();
            s.user = user;
            s.expiresAt = System.currentTimeMillis() + sessionTtlMillis;
            sessions.put(id, s);
            return id;
        }

        public String validate(String id) {
            if (id == null) return null;
            Session s = sessions.get(id);
            if (s == null) return null;
            if (System.currentTimeMillis() > s.expiresAt) {
                sessions.remove(id);
                return null;
            }
            return s.user;
        }

        public boolean terminate(String id) {
            if (id == null) return false;
            return sessions.remove(id) != null;
        }
    }

    // Minimal JSON helpers
    static String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"");
    }

    static Map<String, String> parseJsonObject(String body) {
        Map<String, String> m = new HashMap<>();
        if (body == null) return m;
        int i = 0, n = body.length();
        while (i < n && Character.isWhitespace(body.charAt(i))) i++;
        if (i >= n || body.charAt(i) != '{') return m;
        i++;
        while (i < n) {
            while (i < n && Character.isWhitespace(body.charAt(i))) i++;
            if (i < n && body.charAt(i) == '}') break;
            if (i >= n || body.charAt(i) != '"') break;
            int start = ++i;
            StringBuilder key = new StringBuilder();
            while (i < n && body.charAt(i) != '"') {
                char c = body.charAt(i);
                if (c == '\\' && i + 1 < n) {
                    key.append(body.charAt(i + 1));
                    i += 2;
                } else {
                    key.append(c);
                    i++;
                }
            }
            i++; // skip closing quote
            while (i < n && Character.isWhitespace(body.charAt(i))) i++;
            if (i < n && body.charAt(i) == ':') i++;
            while (i < n && Character.isWhitespace(body.charAt(i))) i++;
            if (i < n && body.charAt(i) == '"') {
                i++;
                StringBuilder val = new StringBuilder();
                while (i < n && body.charAt(i) != '"') {
                    char c = body.charAt(i);
                    if (c == '\\' && i + 1 < n) {
                        val.append(body.charAt(i + 1));
                        i += 2;
                    } else {
                        val.append(c);
                        i++;
                    }
                }
                i++; // closing quote
                m.put(key.toString(), val.toString());
            }
            while (i < n && body.charAt(i) != ',' && body.charAt(i) != '}') i++;
            if (i < n && body.charAt(i) == ',') i++;
        }
        return m;
    }

    // Web application
    static class WebApp {
        final SessionManager sm = new SessionManager();
        final UserStore us = new UserStore();
        final HttpServer server;
        final int port;

        WebApp() throws IOException {
            us.addUser("alice", "p@ssw0rd-Example!");
            server = HttpServer.create(new InetSocketAddress("127.0.0.1", 0), 0);
            server.createContext("/login", this::handleLogin);
            server.createContext("/me", this::handleMe);
            server.createContext("/logout", this::handleLogout);
            server.setExecutor(Executors.newCachedThreadPool());
            server.start();
            port = server.getAddress().getPort();
        }

        void stop() {
            server.stop(0);
        }

        private static void addNoCacheHeaders(Headers h) {
            h.add("Cache-Control", "no-store");
            h.add("X-Content-Type-Options", "nosniff");
        }

        private static String readAll(InputStream is) throws IOException {
            if (is == null) return "";
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            byte[] buf = new byte[4096];
            int r;
            while ((r = is.read(buf)) != -1) bos.write(buf, 0, r);
            return bos.toString(StandardCharsets.UTF_8);
        }

        private static void respondJson(HttpExchange ex, int code, String json) throws IOException {
            byte[] bytes = json.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().add("Content-Type", "application/json; charset=utf-8");
            ex.sendResponseHeaders(code, bytes.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(bytes);
            }
        }

        private static void send400(HttpExchange ex, String msg) throws IOException {
            addNoCacheHeaders(ex.getResponseHeaders());
            respondJson(ex, 400, "{\"error\":\"" + escapeJson(msg) + "\"}");
        }

        private static void send401(HttpExchange ex, String msg) throws IOException {
            addNoCacheHeaders(ex.getResponseHeaders());
            respondJson(ex, 401, "{\"error\":\"" + escapeJson(msg) + "\"}");
        }

        private static void send405(HttpExchange ex) throws IOException {
            addNoCacheHeaders(ex.getResponseHeaders());
            respondJson(ex, 405, "{\"error\":\"Method Not Allowed\"}");
        }

        private static String getCookie(HttpExchange ex, String name) {
            List<String> cookies = ex.getRequestHeaders().get("Cookie");
            if (cookies == null) return null;
            for (String header : cookies) {
                String[] parts = header.split(";");
                for (String part : parts) {
                    String[] nv = part.trim().split("=", 2);
                    if (nv.length == 2 && nv[0].equals(name)) return nv[1];
                }
            }
            return null;
        }

        private static String buildSessionCookie(String sid, boolean expire) {
            StringBuilder sb = new StringBuilder();
            sb.append("SID=").append(sid).append("; Path=/; HttpOnly; SameSite=Strict; Secure");
            if (expire) {
                sb.append("; Max-Age=0");
            } else {
                sb.append("; Max-Age=").append(1800);
            }
            return sb.toString();
        }

        private void handleLogin(HttpExchange ex) throws IOException {
            try {
                if (!"POST".equals(ex.getRequestMethod())) {
                    send405(ex);
                    return;
                }
                String ctype = ex.getRequestHeaders().getFirst("Content-Type");
                if (ctype == null || !ctype.toLowerCase(Locale.ROOT).startsWith("application/json")) {
                    send400(ex, "Invalid Content-Type");
                    return;
                }
                String body = readAll(ex.getRequestBody());
                Map<String, String> json = parseJsonObject(body);
                String username = json.get("username");
                String password = json.get("password");
                if (username == null || password == null) {
                    send400(ex, "Missing fields");
                    return;
                }
                if (!us.verify(username, password)) {
                    send401(ex, "Invalid credentials");
                    return;
                }
                String sid = sm.createSession(username);
                Headers h = ex.getResponseHeaders();
                addNoCacheHeaders(h);
                h.add("Set-Cookie", buildSessionCookie(sid, false));
                respondJson(ex, 200, "{\"status\":\"ok\"}");
            } finally {
                ex.close();
            }
        }

        private void handleMe(HttpExchange ex) throws IOException {
            try {
                if (!"GET".equals(ex.getRequestMethod())) {
                    send405(ex);
                    return;
                }
                String sid = getCookie(ex, "SID");
                String user = sm.validate(sid);
                addNoCacheHeaders(ex.getResponseHeaders());
                if (user == null) {
                    send401(ex, "Not authenticated");
                    return;
                }
                respondJson(ex, 200, "{\"user\":\"" + escapeJson(user) + "\"}");
            } finally {
                ex.close();
            }
        }

        private void handleLogout(HttpExchange ex) throws IOException {
            try {
                if (!"POST".equals(ex.getRequestMethod())) {
                    send405(ex);
                    return;
                }
                String sid = getCookie(ex, "SID");
                if (sid != null) sm.terminate(sid);
                Headers h = ex.getResponseHeaders();
                addNoCacheHeaders(h);
                h.add("Set-Cookie", buildSessionCookie("deleted", true));
                respondJson(ex, 200, "{\"status\":\"logged_out\"}");
            } finally {
                ex.close();
            }
        }
    }

    static class HttpTest {
        static String doPostJson(String url, String json, String cookie) throws Exception {
            URL u = new URL(url);
            HttpURLConnection c = (HttpURLConnection) u.openConnection();
            c.setRequestMethod("POST");
            c.setDoOutput(true);
            c.setRequestProperty("Content-Type", "application/json; charset=utf-8");
            if (cookie != null) c.setRequestProperty("Cookie", cookie);
            byte[] bytes = json.getBytes(StandardCharsets.UTF_8);
            c.setFixedLengthStreamingMode(bytes.length);
            c.connect();
            try (OutputStream os = c.getOutputStream()) {
                os.write(bytes);
            }
            int code = c.getResponseCode();
            String setCookie = c.getHeaderField("Set-Cookie");
            InputStream is = code >= 400 ? c.getErrorStream() : c.getInputStream();
            String resp = "";
            if (is != null) {
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[4096];
                int r;
                while ((r = is.read(buf)) != -1) bos.write(buf, 0, r);
                resp = bos.toString(StandardCharsets.UTF_8);
            }
            return code + "|" + (setCookie == null ? "" : setCookie) + "|" + resp;
        }

        static String doGet(String url, String cookie) throws Exception {
            URL u = new URL(url);
            HttpURLConnection c = (HttpURLConnection) u.openConnection();
            c.setRequestMethod("GET");
            if (cookie != null) c.setRequestProperty("Cookie", cookie);
            int code = c.getResponseCode();
            InputStream is = code >= 400 ? c.getErrorStream() : c.getInputStream();
            String resp = "";
            if (is != null) {
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[4096];
                int r;
                while ((r = is.read(buf)) != -1) bos.write(buf, 0, r);
                resp = bos.toString(StandardCharsets.UTF_8);
            }
            return code + "|" + resp;
        }
    }

    public static void main(String[] args) throws Exception {
        WebApp app = new WebApp();
        int port = app.port;
        String base = "http://127.0.0.1:" + port;

        // Test 1: GET /me without cookie -> 401
        System.out.println("TEST1:" + HttpTest.doGet(base + "/me", null));

        // Test 2: POST /login with wrong password -> 401
        System.out.println("TEST2:" + HttpTest.doPostJson(base + "/login", "{\"username\":\"alice\",\"password\":\"wrong\"}", null));

        // Test 3: POST /login with correct password -> 200 and Set-Cookie
        String r3 = HttpTest.doPostJson(base + "/login", "{\"username\":\"alice\",\"password\":\"p@ssw0rd-Example!\"}", null);
        System.out.println("TEST3:" + r3);
        String cookie = null;
        int p1 = r3.indexOf("|") + 1;
        int p2 = r3.indexOf("|", p1);
        String setCookie = r3.substring(p1, p2);
        if (setCookie != null && setCookie.startsWith("SID=")) {
            int scEnd = setCookie.indexOf(';');
            cookie = scEnd >= 0 ? setCookie.substring(0, scEnd) : setCookie;
        }

        // Test 4: GET /me with cookie -> 200
        System.out.println("TEST4:" + HttpTest.doGet(base + "/me", cookie));

        // Test 5: POST /logout -> 200; then GET /me -> 401
        System.out.println("TEST5-1:" + HttpTest.doPostJson(base + "/logout", "{}", cookie));
        System.out.println("TEST5-2:" + HttpTest.doGet(base + "/me", cookie));

        app.stop();
    }
}