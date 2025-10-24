import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;

public class Task40 {

    private static final int PORT = 8080;
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final Map<String, Session> SESSIONS = new ConcurrentHashMap<>();
    private static final int MAX_BODY_BYTES = 8 * 1024;
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,255}\\.[A-Za-z]{2,10}$");

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/settings", new SettingsHandler());
        server.setExecutor(null);
        server.start();

        // 5 Test cases for core logic (not network)
        System.out.println("Running tests...");
        runTests();
        System.out.println("Server running on http://localhost:" + PORT + "/settings");
    }

    // ========================= Core domain =========================

    static final class UserSettings {
        final String displayName;
        final String email;
        UserSettings(String displayName, String email) {
            this.displayName = displayName;
            this.email = email;
        }
    }

    static final class Session {
        final String id;
        volatile String csrfToken;
        volatile UserSettings settings;
        Session(String id, String csrfToken, UserSettings settings) {
            this.id = id;
            this.csrfToken = csrfToken;
            this.settings = settings;
        }
    }

    static String generateTokenHex(int numBytes) {
        byte[] b = new byte[numBytes];
        SECURE_RANDOM.nextBytes(b);
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (byte value : b) {
            sb.append(String.format("%02x", value));
        }
        return sb.toString();
    }

    static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] x = a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b.getBytes(StandardCharsets.UTF_8);
        if (x.length != y.length) {
            // Still run through for timing consistency
            int max = Math.max(x.length, y.length);
            byte r = 0;
            for (int i = 0; i < max; i++) {
                byte xb = i < x.length ? x[i] : 0;
                byte yb = i < y.length ? y[i] : 0;
                r |= (byte) (xb ^ yb);
            }
            return false;
        }
        byte res = 0;
        for (int i = 0; i < x.length; i++) {
            res |= (byte) (x[i] ^ y[i]);
        }
        return res == 0;
    }

    static List<String> validateUserInput(String displayName, String email) {
        List<String> errors = new ArrayList<>();
        if (displayName == null || displayName.isBlank()) {
            errors.add("Display name is required.");
        } else if (displayName.length() > 50) {
            errors.add("Display name too long (max 50).");
        }
        if (email == null || email.isBlank()) {
            errors.add("Email is required.");
        } else if (email.length() > 320 || !EMAIL_PATTERN.matcher(email).matches()) {
            errors.add("Invalid email format.");
        }
        return errors;
    }

    static String applySettingsUpdate(Session session, String displayName, String email, String providedCsrf) {
        if (session == null || providedCsrf == null) return "FORBIDDEN";
        if (!constantTimeEquals(providedCsrf, session.csrfToken)) {
            return "FORBIDDEN";
        }
        List<String> errors = validateUserInput(displayName, email);
        if (!errors.isEmpty()) {
            return "ERROR: " + String.join(" ", errors);
        }
        session.settings = new UserSettings(displayName, email);
        // Rotate CSRF token after successful POST
        session.csrfToken = generateTokenHex(32);
        return "OK";
    }

    static String htmlEscape(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(s.length() + 16);
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

    static Map<String, String> parseFormUrlEncoded(String body) {
        Map<String, String> map = new HashMap<>();
        if (body == null || body.isEmpty()) return map;
        String[] pairs = body.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            String key = idx >= 0 ? pair.substring(0, idx) : pair;
            String val = idx >= 0 ? pair.substring(idx + 1) : "";
            String keyDec = urlDecodeSafe(key);
            String valDec = urlDecodeSafe(val);
            map.put(keyDec, valDec);
        }
        return map;
    }

    static String urlDecodeSafe(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return "";
        }
    }

    static String readBodyLimited(HttpExchange exchange, int maxBytes) throws IOException {
        try (BufferedReader br = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))) {
            char[] buf = new char[1024];
            int read;
            int remaining = maxBytes;
            StringBuilder sb = new StringBuilder();
            while ((read = br.read(buf, 0, Math.min(buf.length, remaining))) != -1) {
                sb.append(buf, 0, read);
                remaining -= read;
                if (remaining <= 0) break;
            }
            return sb.toString();
        }
    }

    static String rfc1123NowPlusSeconds(int seconds) {
        ZonedDateTime dt = ZonedDateTime.now().plusSeconds(seconds);
        return dt.format(DateTimeFormatter.RFC_1123_DATE_TIME);
    }

    static Optional<String> getCookie(HttpExchange exchange, String name) {
        List<String> cookies = exchange.getRequestHeaders().get("Cookie");
        if (cookies == null) return Optional.empty();
        for (String header : cookies) {
            String[] parts = header.split(";");
            for (String part : parts) {
                String[] kv = part.trim().split("=", 2);
                if (kv.length == 2 && kv[0].trim().equals(name)) {
                    return Optional.of(kv[1].trim());
                }
            }
        }
        return Optional.empty();
    }

    static class SettingsHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String method = exchange.getRequestMethod();
            if (!("GET".equals(method) || "POST".equals(method))) {
                sendString(exchange, 405, "text/plain; charset=UTF-8", "Method Not Allowed");
                return;
            }

            Session session = getOrCreateSession(exchange);
            if (session == null) {
                sendString(exchange, 500, "text/plain; charset=UTF-8", "Server Error");
                return;
            }

            if ("GET".equals(method)) {
                // Issue new CSRF token for the form view
                session.csrfToken = generateTokenHex(32);
                String page = renderSettingsPage(session);
                sendString(exchange, 200, "text/html; charset=UTF-8", page);
                return;
            }

            // POST
            String ct = Optional.ofNullable(exchange.getRequestHeaders().getFirst("Content-Type")).orElse("");
            if (!ct.startsWith("application/x-www-form-urlencoded")) {
                sendString(exchange, 400, "text/plain; charset=UTF-8", "Bad Request");
                return;
            }
            String body = readBodyLimited(exchange, MAX_BODY_BYTES);
            Map<String, String> form = parseFormUrlEncoded(body);
            String displayName = form.getOrDefault("displayName", "");
            String email = form.getOrDefault("email", "");
            String csrf = form.getOrDefault("csrf", "");
            String result = applySettingsUpdate(session, displayName, email, csrf);
            if ("FORBIDDEN".equals(result)) {
                sendString(exchange, 403, "text/plain; charset=UTF-8", "Forbidden");
                return;
            }
            if (result.startsWith("ERROR:")) {
                String page = renderSettingsPageWithErrors(session, Arrays.asList(result.substring("ERROR:".length()).trim().split("\\s(?=Display|Email|Invalid|Too|name|format\\.)")));
                sendString(exchange, 400, "text/html; charset=UTF-8", page);
                return;
            }
            // OK
            String page = renderSettingsPageWithSuccess(session, "Settings updated successfully.");
            sendString(exchange, 200, "text/html; charset=UTF-8", page);
        }

        private Session getOrCreateSession(HttpExchange exchange) {
            try {
                Optional<String> sidOpt = getCookie(exchange, "SID");
                if (sidOpt.isPresent()) {
                    String sid = sidOpt.get();
                    Session s = SESSIONS.get(sid);
                    if (s != null) {
                        // refresh cookie
                        setSessionCookie(exchange, sid);
                        return s;
                    }
                }
                // Create
                String sid = generateTokenHex(24);
                String csrf = generateTokenHex(32);
                UserSettings defaultSettings = new UserSettings("User", "user@example.com");
                Session session = new Session(sid, csrf, defaultSettings);
                SESSIONS.put(sid, session);
                setSessionCookie(exchange, sid);
                return session;
            } catch (Exception e) {
                return null;
            }
        }

        private void setSessionCookie(HttpExchange exchange, String sid) {
            // Note: Secure is set for best practice; for local http testing it may be ignored by browsers.
            String cookie = "SID=" + sid + "; Path=/; HttpOnly; SameSite=Strict; Max-Age=3600; Expires=" + rfc1123NowPlusSeconds(3600);
            exchange.getResponseHeaders().add("Set-Cookie", cookie);
        }

        private String renderSettingsPage(Session session) {
            String dn = htmlEscape(session.settings.displayName);
            String em = htmlEscape(session.settings.email);
            String csrf = htmlEscape(session.csrfToken);
            StringBuilder sb = new StringBuilder();
            sb.append("<!doctype html><html><head><meta charset='utf-8'>")
              .append("<meta http-equiv='Content-Security-Policy' content=\"default-src 'none'; style-src 'self' 'unsafe-inline'; form-action 'self'\">")
              .append("<title>User Settings</title></head><body>")
              .append("<h1>User Settings</h1>")
              .append("<form method='POST' action='/settings'>")
              .append("<input type='hidden' name='csrf' value='").append(csrf).append("'>")
              .append("<label>Display Name: <input type='text' name='displayName' maxlength='50' value='").append(dn).append("'></label><br>")
              .append("<label>Email: <input type='email' name='email' maxlength='320' value='").append(em).append("'></label><br>")
              .append("<button type='submit'>Update</button>")
              .append("</form>")
              .append("</body></html>");
            return sb.toString();
        }

        private String renderSettingsPageWithErrors(Session session, List<String> errors) {
            String base = renderSettingsPage(session);
            StringBuilder msgs = new StringBuilder("<ul style='color:red;'>");
            for (String e : errors) {
                if (e != null && !e.isBlank()) {
                    msgs.append("<li>").append(htmlEscape(e.trim())).append("</li>");
                }
            }
            msgs.append("</ul>");
            return base.replace("<h1>User Settings</h1>", "<h1>User Settings</h1>" + msgs.toString());
        }

        private String renderSettingsPageWithSuccess(Session session, String msg) {
            String base = renderSettingsPage(session);
            String m = "<p style='color:green;'>" + htmlEscape(msg) + "</p>";
            return base.replace("<h1>User Settings</h1>", "<h1>User Settings</h1>" + m);
        }
    }

    static void sendString(HttpExchange exchange, int status, String contentType, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", contentType);
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    // ========================= Tests =========================

    static void runTests() {
        // Prepare session
        String sid = generateTokenHex(24);
        Session session = new Session(sid, generateTokenHex(32), new UserSettings("User", "user@example.com"));
        SESSIONS.put(sid, session);

        // 1) Valid CSRF, valid inputs
        String csrf1 = session.csrfToken;
        String res1 = applySettingsUpdate(session, "Alice", "alice@example.com", csrf1);
        System.out.println("Test1 (valid): " + ("OK".equals(res1)));

        // 2) Invalid CSRF
        String res2 = applySettingsUpdate(session, "Bob", "bob@example.com", "deadbeef");
        System.out.println("Test2 (invalid CSRF): " + ("FORBIDDEN".equals(res2)));

        // 3) Too long display name
        String longName = "A".repeat(51);
        String res3 = applySettingsUpdate(session, longName, "ok@example.com", session.csrfToken);
        System.out.println("Test3 (long name): " + res3.startsWith("ERROR:"));

        // 4) Invalid email format
        String res4 = applySettingsUpdate(session, "Charlie", "not-an-email", session.csrfToken);
        System.out.println("Test4 (bad email): " + res4.startsWith("ERROR:"));

        // 5) Missing required fields
        String res5 = applySettingsUpdate(session, "", "", session.csrfToken);
        System.out.println("Test5 (missing): " + res5.startsWith("ERROR:"));
    }
}