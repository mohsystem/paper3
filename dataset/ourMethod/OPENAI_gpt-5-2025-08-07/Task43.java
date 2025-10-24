import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

class Task43 {

    // Data Models
    static final class User {
        final String username;
        final byte[] salt;
        final byte[] hash;
        final long createdAtEpochSec;

        User(String username, byte[] salt, byte[] hash, long createdAtEpochSec) {
            this.username = username;
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
            this.createdAtEpochSec = createdAtEpochSec;
        }
    }

    static final class Session {
        final String token;
        final String username;
        final long expiresAtEpochSec;

        Session(String token, String username, long expiresAtEpochSec) {
            this.token = token;
            this.username = username;
            this.expiresAtEpochSec = expiresAtEpochSec;
        }
    }

    static final class Request {
        final String method;
        final String path;
        final Map<String, String> headers;
        final Map<String, String> body;

        Request(String method, String path, Map<String, String> headers, Map<String, String> body) {
            this.method = method;
            this.path = path;
            this.headers = new HashMap<>(headers);
            this.body = new HashMap<>(body);
        }
    }

    static final class Response {
        final int status;
        final Map<String, String> headers;
        final String body;

        Response(int status, Map<String, String> headers, String body) {
            this.status = status;
            this.headers = new HashMap<>(headers);
            this.body = body;
        }
    }

    // Utilities
    static final class Hex {
        static String toHex(byte[] b) {
            StringBuilder sb = new StringBuilder(b.length * 2);
            for (byte x : b) {
                sb.append(String.format("%02x", x));
            }
            return sb.toString();
        }
    }

    static final class PasswordPolicy {
        static boolean isValid(String password) {
            if (password == null) return false;
            if (password.length() < 12 || password.length() > 128) return false;
            boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
            for (char c : password.toCharArray()) {
                if (Character.isUpperCase(c)) hasUpper = true;
                else if (Character.isLowerCase(c)) hasLower = true;
                else if (Character.isDigit(c)) hasDigit = true;
                else hasSpecial = true;
            }
            return hasUpper && hasLower && hasDigit && hasSpecial;
        }
    }

    static final class Validators {
        static boolean validUsername(String username) {
            if (username == null) return false;
            if (username.length() < 3 || username.length() > 32) return false;
            for (char c : username.toCharArray()) {
                if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-')) return false;
            }
            return true;
        }
    }

    // Security constants
    static final class SecurityParams {
        static final int SALT_LEN = 16;
        static final int HASH_LEN = 32;
        static final int PBKDF2_ITERATIONS = 210_000;
        static final long SESSION_TTL_SECONDS = 1800; // 30 minutes
        static final long PASSWORD_MAX_AGE_SECONDS = 90L * 24 * 3600; // 90 days
    }

    // Session Manager
    static final class SessionManager {
        private final Map<String, User> users = new HashMap<>();
        private final Map<String, Session> sessions = new HashMap<>();
        private final SecureRandom rng = new SecureRandom();

        public synchronized Map<String, Object> registerUser(String username, String password) {
            Map<String, Object> res = new HashMap<>();
            if (!Validators.validUsername(username)) {
                res.put("ok", false);
                res.put("error", "Invalid username");
                return res;
            }
            if (!PasswordPolicy.isValid(password)) {
                res.put("ok", false);
                res.put("error", "Weak password");
                return res;
            }
            if (users.containsKey(username)) {
                res.put("ok", false);
                res.put("error", "User exists");
                return res;
            }
            byte[] salt = new byte[SecurityParams.SALT_LEN];
            rng.nextBytes(salt);
            byte[] hash = hashPassword(password, salt);
            long now = Instant.now().getEpochSecond();
            users.put(username, new User(username, salt, hash, now));
            res.put("ok", true);
            return res;
        }

        public synchronized Map<String, Object> login(String username, String password) {
            Map<String, Object> res = new HashMap<>();
            User user = users.get(username);
            if (user == null) {
                res.put("ok", false);
                res.put("error", "Invalid credentials");
                return res;
            }
            if (!verifyPassword(password, user.salt, user.hash)) {
                res.put("ok", false);
                res.put("error", "Invalid credentials");
                return res;
            }
            long now = Instant.now().getEpochSecond();
            if (now - user.createdAtEpochSec > SecurityParams.PASSWORD_MAX_AGE_SECONDS) {
                res.put("ok", false);
                res.put("error", "Password expired");
                return res;
            }
            String token = generateToken();
            Session session = new Session(token, username, now + SecurityParams.SESSION_TTL_SECONDS);
            sessions.put(token, session);
            res.put("ok", true);
            res.put("token", token);
            return res;
        }

        public synchronized Map<String, Object> me(String token) {
            Map<String, Object> res = new HashMap<>();
            cleanupExpiredSessions();
            if (token == null) {
                res.put("ok", false);
                res.put("error", "No session");
                return res;
            }
            Session s = sessions.get(token);
            if (s == null || s.expiresAtEpochSec < Instant.now().getEpochSecond()) {
                res.put("ok", false);
                res.put("error", "Invalid or expired session");
                return res;
            }
            res.put("ok", true);
            res.put("username", s.username);
            return res;
        }

        public synchronized Map<String, Object> logout(String token) {
            Map<String, Object> res = new HashMap<>();
            if (token != null) {
                sessions.remove(token);
            }
            res.put("ok", true);
            return res;
        }

        public synchronized void cleanupExpiredSessions() {
            long now = Instant.now().getEpochSecond();
            Iterator<Map.Entry<String, Session>> it = sessions.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry<String, Session> e = it.next();
                if (e.getValue().expiresAtEpochSec < now) it.remove();
            }
        }

        private String generateToken() {
            byte[] b = new byte[32];
            rng.nextBytes(b);
            return Hex.toHex(b);
        }

        private byte[] hashPassword(String password, byte[] salt) {
            try {
                PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, SecurityParams.PBKDF2_ITERATIONS, SecurityParams.HASH_LEN * 8);
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                return skf.generateSecret(spec).getEncoded();
            } catch (Exception e) {
                throw new IllegalStateException("KDF error");
            }
        }

        private boolean verifyPassword(String password, byte[] salt, byte[] expectedHash) {
            byte[] h = hashPassword(password, salt);
            return MessageDigest.isEqual(h, expectedHash);
        }
    }

    // "Web" layer simulation
    static final class WebApp {
        private final SessionManager mgr;

        WebApp(SessionManager mgr) {
            this.mgr = mgr;
        }

        public Response handle(Request req) {
            switch (req.path) {
                case "/register":
                    if (!"POST".equals(req.method)) return json(405, "Method Not Allowed");
                    String ru = req.body.getOrDefault("username", "");
                    String rp = req.body.getOrDefault("password", "");
                    Map<String, Object> r = mgr.registerUser(ru, rp);
                    if (Boolean.TRUE.equals(r.get("ok"))) {
                        return json(201, "{\"status\":\"registered\"}");
                    } else {
                        return json(400, "{\"error\":\"" + safeJson(r.get("error")) + "\"}");
                    }
                case "/login":
                    if (!"POST".equals(req.method)) return json(405, "Method Not Allowed");
                    String lu = req.body.getOrDefault("username", "");
                    String lp = req.body.getOrDefault("password", "");
                    Map<String, Object> l = mgr.login(lu, lp);
                    if (Boolean.TRUE.equals(l.get("ok"))) {
                        String token = (String) l.get("token");
                        Map<String, String> headers = new HashMap<>();
                        headers.put("Set-Cookie", "SID=" + token + "; HttpOnly; Secure; SameSite=Strict; Path=/");
                        return new Response(200, headers, "{\"status\":\"ok\"}");
                    } else {
                        return json(401, "{\"error\":\"" + safeJson(l.get("error")) + "\"}");
                    }
                case "/me":
                    if (!"GET".equals(req.method)) return json(405, "Method Not Allowed");
                    String cookie = req.headers.getOrDefault("Cookie", "");
                    String sid = parseCookie(cookie, "SID");
                    Map<String, Object> m = mgr.me(sid);
                    if (Boolean.TRUE.equals(m.get("ok"))) {
                        String un = (String) m.get("username");
                        return json(200, "{\"username\":\"" + safeJson(un) + "\"}");
                    } else {
                        return json(401, "{\"error\":\"" + safeJson(m.get("error")) + "\"}");
                    }
                case "/logout":
                    if (!"POST".equals(req.method)) return json(405, "Method Not Allowed");
                    String cookie2 = req.headers.getOrDefault("Cookie", "");
                    String sid2 = parseCookie(cookie2, "SID");
                    mgr.logout(sid2);
                    Map<String, String> headers = new HashMap<>();
                    headers.put("Set-Cookie", "SID=; HttpOnly; Secure; SameSite=Strict; Path=/; Max-Age=0");
                    return new Response(200, headers, "{\"status\":\"logged_out\"}");
                default:
                    return json(404, "{\"error\":\"not_found\"}");
            }
        }

        private static String parseCookie(String cookie, String name) {
            if (cookie == null) return null;
            String[] parts = cookie.split(";");
            for (String p : parts) {
                String[] kv = p.trim().split("=", 2);
                if (kv.length == 2 && kv[0].equals(name)) {
                    return kv[1];
                }
            }
            return null;
        }

        private static Response json(int status, String body) {
            Map<String, String> h = new HashMap<>();
            h.put("Content-Type", "application/json; charset=utf-8");
            return new Response(status, h, body);
        }

        private static String safeJson(Object o) {
            if (o == null) return "";
            String s = String.valueOf(o);
            return s.replace("\\", "\\\\").replace("\"", "\\\"");
        }
    }

    public static void main(String[] args) {
        SessionManager mgr = new SessionManager();
        WebApp app = new WebApp(mgr);

        // Test 1: Register user
        Response r1 = app.handle(new Request("POST", "/register",
                Collections.emptyMap(),
                Map.of("username", "alice_1", "password", "Str0ng!Passw0rd")));
        System.out.println("T1 status=" + r1.status + " body=" + r1.body);

        // Test 2: Weak password
        Response r2 = app.handle(new Request("POST", "/register",
                Collections.emptyMap(),
                Map.of("username", "bob", "password", "weakpass")));
        System.out.println("T2 status=" + r2.status + " body=" + r2.body);

        // Test 3: Login
        Response r3 = app.handle(new Request("POST", "/login",
                Collections.emptyMap(),
                Map.of("username", "alice_1", "password", "Str0ng!Passw0rd")));
        String setCookie = r3.headers.getOrDefault("Set-Cookie", "");
        System.out.println("T3 status=" + r3.status + " set-cookie-present=" + (!setCookie.isEmpty()));

        // Test 4: Access /me
        Response r4 = app.handle(new Request("GET", "/me",
                Map.of("Cookie", setCookie.split(";", 2)[0]), // only SID=...
                Collections.emptyMap()));
        System.out.println("T4 status=" + r4.status + " body=" + r4.body);

        // Test 5: Logout then /me should fail
        Response r5 = app.handle(new Request("POST", "/logout",
                Map.of("Cookie", setCookie.split(";", 2)[0]),
                Collections.emptyMap()));
        Response r6 = app.handle(new Request("GET", "/me",
                Map.of("Cookie", setCookie.split(";", 2)[0]),
                Collections.emptyMap()));
        System.out.println("T5 logout-status=" + r5.status + " me-after-logout-status=" + r6.status);
    }
}