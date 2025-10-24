import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

/*
 Web application in Java with CSRF protection.
 - Starts an HTTP server on port 8080
 - Endpoints:
   GET  /settings  -> Renders a form to update user settings (display name, email notifications)
   POST /update    -> Processes the form, validates CSRF token and Origin/Referer, updates settings
 Security measures:
   - Session management via HttpOnly, SameSite=Strict cookie
   - Per-session CSRF token, rotated on form render and after successful POST
   - Validates Origin/Referer against server origin
   - Limits request size, enforces Content-Type
   - Sets basic security headers
   - Escapes HTML outputs and validates inputs
 Note:
   - For local testing over HTTP, Secure attribute is omitted from cookie. In production, add "; Secure" and use HTTPS.
*/

public class Task40 {

    // Session structure
    static class Session {
        final String id;
        String csrfToken;
        String displayName = "Anonymous";
        boolean emailNotif = false;
        long lastAccess = Instant.now().getEpochSecond();

        Session(String id) {
            this.id = id;
        }
    }

    // In-memory session store
    private static final Map<String, Session> SESSIONS = new ConcurrentHashMap<>();
    private static final SecureRandom RNG = new SecureRandom();
    private static final int MAX_BODY = 8 * 1024; // 8 KiB limit
    private static final int TOKEN_BYTES = 32;

    // Generate a URL-safe CSRF token with given number of random bytes
    public static String generateToken(int numBytes) {
        byte[] buf = new byte[numBytes];
        RNG.nextBytes(buf);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
    }

    // Validate CSRF token equality in a constant-time manner
    public static boolean validateCsrf(String sessionToken, String providedToken) {
        if (sessionToken == null || providedToken == null) return false;
        if (sessionToken.length() != providedToken.length()) {
            // Normalize lengths to avoid timing side-channels
            // by comparing strings of equal length.
            int max = Math.max(sessionToken.length(), providedToken.length());
            String a = padRight(sessionToken, max);
            String b = padRight(providedToken, max);
            return constantTimeEquals(a, b) && false;
        }
        return constantTimeEquals(sessionToken, providedToken);
    }

    private static String padRight(String s, int len) {
        StringBuilder sb = new StringBuilder(len);
        sb.append(s);
        while (sb.length() < len) sb.append('\0');
        return sb.toString();
    }

    private static boolean constantTimeEquals(String a, String b) {
        byte[] x = a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b.getBytes(StandardCharsets.UTF_8);
        int diff = x.length ^ y.length;
        for (int i = 0; i < x.length && i < y.length; i++) {
            diff |= (x[i] ^ y[i]);
        }
        return diff == 0;
    }

    // HTML escape to avoid XSS
    public static String htmlEscape(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder();
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                case '/': out.append("&#47;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    // Build the settings form HTML
    public static String buildSettingsForm(String csrfToken, String displayName, boolean emailNotif) {
        String dn = htmlEscape(displayName);
        String checked = emailNotif ? "checked" : "";
        return "<!doctype html><html><head>" +
                "<meta charset=\"utf-8\">" +
                "<title>User Settings</title>" +
                "</head><body>" +
                "<h1>Update Settings</h1>" +
                "<form method=\"POST\" action=\"/update\">" +
                "<label for=\"displayName\">Display Name:</label> " +
                "<input id=\"displayName\" name=\"displayName\" maxlength=\"50\" value=\"" + dn + "\" required>" +
                "<br><label><input type=\"checkbox\" name=\"emailNotif\" value=\"1\" " + checked + "> Email Notifications</label>" +
                "<input type=\"hidden\" name=\"_csrf\" value=\"" + htmlEscape(csrfToken) + "\">" +
                "<br><button type=\"submit\">Save</button>" +
                "</form>" +
                "<p>CSRF token is bound to your session and validated on submit.</p>" +
                "</body></html>";
    }

    // Parse application/x-www-form-urlencoded body into Map
    public static Map<String, String> parseForm(String body) throws UnsupportedEncodingException {
        Map<String, String> map = new HashMap<>();
        if (body == null || body.isEmpty()) return map;
        String[] pairs = body.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            String key, value;
            if (idx >= 0) {
                key = pair.substring(0, idx);
                value = pair.substring(idx + 1);
            } else {
                key = pair;
                value = "";
            }
            key = URLDecoder.decode(key, StandardCharsets.UTF_8.name());
            value = URLDecoder.decode(value, StandardCharsets.UTF_8.name());
            map.put(key, value);
        }
        return map;
    }

    // Get or create session from cookies; sets Set-Cookie if new
    private static Session getOrCreateSession(HttpExchange ex) {
        String sid = null;
        Headers req = ex.getRequestHeaders();
        List<String> cookies = req.getOrDefault("Cookie", Collections.emptyList());
        for (String header : cookies) {
            String[] parts = header.split(";");
            for (String part : parts) {
                String[] kv = part.trim().split("=", 2);
                if (kv.length == 2) {
                    if (kv[0].equals("SID")) {
                        sid = kv[1];
                    }
                }
            }
        }
        Session session = null;
        if (sid != null) {
            session = SESSIONS.get(sid);
        }
        if (session == null) {
            String newSid = generateToken(24);
            session = new Session(newSid);
            SESSIONS.put(newSid, session);
            // HttpOnly + SameSite=Strict; omit Secure for local http testing
            String cookie = "SID=" + newSid + "; Path=/; HttpOnly; SameSite=Strict; Max-Age=86400";
            ex.getResponseHeaders().add("Set-Cookie", cookie);
        } else {
            session.lastAccess = Instant.now().getEpochSecond();
        }
        return session;
    }

    // Adds minimal secure headers
    private static void addSecurityHeaders(Headers h) {
        h.set("Content-Type", "text/html; charset=utf-8");
        h.set("X-Content-Type-Options", "nosniff");
        h.set("X-Frame-Options", "DENY");
        h.set("Referrer-Policy", "no-referrer");
        h.set("Cache-Control", "no-store");
        h.set("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'; form-action 'self'; frame-ancestors 'none'");
    }

    // Validate Origin or Referer matches our origin
    private static boolean originAllowed(HttpExchange ex) {
        String host = firstHeader(ex.getRequestHeaders(), "Host");
        String scheme = "http"; // For local demo; if behind TLS, set https
        String expectedOrigin = scheme + "://" + (host != null ? host : "localhost:8080");

        String origin = firstHeader(ex.getRequestHeaders(), "Origin");
        String referer = firstHeader(ex.getRequestHeaders(), "Referer");
        if (origin != null) {
            return origin.equalsIgnoreCase(expectedOrigin);
        }
        if (referer != null) {
            // Check prefix; Referer may include path
            return referer.toLowerCase(Locale.ROOT).startsWith(expectedOrigin.toLowerCase(Locale.ROOT) + "/");
        }
        // No Origin or Referer -> reject to be strict
        return false;
    }

    private static String firstHeader(Headers headers, String name) {
        List<String> vals = headers.get(name);
        if (vals == null || vals.isEmpty()) return null;
        return vals.get(0);
    }

    private static byte[] readBodyLimited(InputStream is, int limit) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[2048];
        int total = 0;
        int r;
        while ((r = is.read(buf)) != -1) {
            total += r;
            if (total > limit) {
                throw new IOException("Request entity too large");
            }
            bos.write(buf, 0, r);
            if (r == 0) break;
        }
        return bos.toByteArray();
    }

    // Input validation for display name
    private static String sanitizeDisplayName(String s) {
        if (s == null) return "Anonymous";
        s = s.trim();
        if (s.length() > 50) s = s.substring(0, 50);
        if (!s.matches("[\\p{L}\\p{N} _\\-]{1,50}")) {
            // If invalid, fallback to safe default
            return "Anonymous";
        }
        return s;
    }

    // HTTP Handlers
    static class SettingsHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(ex.getRequestMethod())) {
                    sendPlain(ex, 405, "Method Not Allowed");
                    return;
                }
                Session session = getOrCreateSession(ex);
                // Rotate CSRF token for each render
                session.csrfToken = generateToken(TOKEN_BYTES);
                String html = buildSettingsForm(session.csrfToken, session.displayName, session.emailNotif);
                Headers rh = ex.getResponseHeaders();
                addSecurityHeaders(rh);
                byte[] out = html.getBytes(StandardCharsets.UTF_8);
                ex.sendResponseHeaders(200, out.length);
                try (OutputStream os = ex.getResponseBody()) { os.write(out); }
            } catch (Exception e) {
                sendPlain(ex, 500, "Internal Server Error");
            } finally {
                ex.close();
            }
        }
    }

    static class UpdateHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(ex.getRequestMethod())) {
                    sendPlain(ex, 405, "Method Not Allowed");
                    return;
                }
                // Enforce content type
                String ct = firstHeader(ex.getRequestHeaders(), "Content-Type");
                if (ct == null || !ct.toLowerCase(Locale.ROOT).startsWith("application/x-www-form-urlencoded")) {
                    sendPlain(ex, 415, "Unsupported Media Type");
                    return;
                }
                if (!originAllowed(ex)) {
                    sendPlain(ex, 403, "Forbidden");
                    return;
                }
                Session session = getOrCreateSession(ex);
                byte[] body = readBodyLimited(ex.getRequestBody(), MAX_BODY);
                Map<String, String> form = parseForm(new String(body, StandardCharsets.UTF_8));

                String token = form.get("_csrf");
                if (!validateCsrf(session.csrfToken, token)) {
                    sendPlain(ex, 403, "Forbidden");
                    return;
                }

                String newName = sanitizeDisplayName(form.get("displayName"));
                boolean newEmail = "1".equals(form.get("emailNotif"));

                session.displayName = newName;
                session.emailNotif = newEmail;
                // Rotate token after successful POST to prevent replay
                session.csrfToken = generateToken(TOKEN_BYTES);

                String response = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Saved</title></head><body>" +
                        "<h2>Settings updated securely.</h2>" +
                        "<p>Display Name: " + htmlEscape(session.displayName) + "</p>" +
                        "<p>Email Notifications: " + (session.emailNotif ? "Enabled" : "Disabled") + "</p>" +
                        "<p><a href=\"/settings\">Back to Settings</a></p>" +
                        "</body></html>";
                Headers rh = ex.getResponseHeaders();
                addSecurityHeaders(rh);
                byte[] out = response.getBytes(StandardCharsets.UTF_8);
                ex.sendResponseHeaders(200, out.length);
                try (OutputStream os = ex.getResponseBody()) { os.write(out); }
            } catch (IOException e) {
                sendPlain(ex, 413, "Payload Too Large");
            } catch (Exception e) {
                sendPlain(ex, 500, "Internal Server Error");
            } finally {
                ex.close();
            }
        }
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(ex.getRequestMethod())) {
                    sendPlain(ex, 405, "Method Not Allowed");
                    return;
                }
                Session session = getOrCreateSession(ex);
                String html = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Home</title></head><body>" +
                        "<h1>Welcome</h1>" +
                        "<p>Hello, " + htmlEscape(session.displayName) + "</p>" +
                        "<p><a href=\"/settings\">Go to Settings</a></p>" +
                        "</body></html>";
                Headers rh = ex.getResponseHeaders();
                addSecurityHeaders(rh);
                byte[] out = html.getBytes(StandardCharsets.UTF_8);
                ex.sendResponseHeaders(200, out.length);
                try (OutputStream os = ex.getResponseBody()) { os.write(out); }
            } catch (Exception e) {
                sendPlain(ex, 500, "Internal Server Error");
            } finally {
                ex.close();
            }
        }
    }

    private static void sendPlain(HttpExchange ex, int code, String msg) throws IOException {
        Headers rh = ex.getResponseHeaders();
        rh.set("Content-Type", "text/plain; charset=utf-8");
        rh.set("X-Content-Type-Options", "nosniff");
        byte[] out = msg.getBytes(StandardCharsets.UTF_8);
        ex.sendResponseHeaders(code, out.length);
        try (OutputStream os = ex.getResponseBody()) { os.write(out); }
    }

    // Simple tests
    private static void runTests() throws Exception {
        System.out.println("[Java] Running tests...");
        // 1) Token uniqueness
        Set<String> tokens = new HashSet<>();
        for (int i = 0; i < 100; i++) {
            String t = generateToken(16);
            if (!tokens.add(t)) throw new RuntimeException("Token collision");
        }
        System.out.println("Test 1 OK: Tokens are unique");

        // 2) HTML escape
        String escaped = htmlEscape("<script>alert('x')</script>&");
        if (!escaped.contains("&lt;script&gt;") || !escaped.contains("&amp;")) {
            throw new RuntimeException("HTML escape failed");
        }
        System.out.println("Test 2 OK: HTML escape works");

        // 3) Form parsing
        Map<String, String> parsed = parseForm("a=1&b=2%2B3&c=");
        if (!"1".equals(parsed.get("a")) || !"2+3".equals(parsed.get("b")) || !"".equals(parsed.get("c"))) {
            throw new RuntimeException("Form parsing failed");
        }
        System.out.println("Test 3 OK: Form parsing works");

        // 4) CSRF validation success
        String t = generateToken(16);
        if (!validateCsrf(t, t)) throw new RuntimeException("CSRF validation should succeed");
        System.out.println("Test 4 OK: CSRF validation success");

        // 5) CSRF validation failure
        if (validateCsrf(generateToken(16), generateToken(16))) throw new RuntimeException("CSRF validation should fail");
        System.out.println("Test 5 OK: CSRF validation failure");
    }

    public static void main(String[] args) throws Exception {
        runTests();

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/settings", new SettingsHandler());
        server.createContext("/update", new UpdateHandler());
        server.setExecutor(null);
        System.out.println("Java server started at http://localhost:8080");
        server.start();
    }
}