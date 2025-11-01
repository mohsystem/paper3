import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Arrays;
import java.util.Base64;
import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Lse40 {
    private static final int PORT = 8080;
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final Map<String, SessionData> SESSIONS = new ConcurrentHashMap<>();

    private static final class SessionData {
        final String username;
        final String email;
        final String ssn;
        SessionData(String username, String email, String ssn) {
            this.username = username;
            this.email = email;
            this.ssn = ssn;
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for renderInfoPage
        System.out.println(renderInfoPage("alice", "alice@example.com", "123-45-6789"));
        System.out.println(renderInfoPage("bob<script>", "bob@example.com", "000-00-0001"));
        System.out.println(renderInfoPage("carol", "carol+test@example.com", "9999"));
        System.out.println(renderInfoPage("dave", "dave.o'reilly@example.com", "12-3456"));
        System.out.println(renderInfoPage("eve", "eve@example.com", "abcd-ef-ghij"));

        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", PORT), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server started on http://127.0.0.1:" + PORT + "/info");
    }

    private static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendResponse(exchange, 405, "Method Not Allowed");
                    return;
                }
                if (!"/info".equals(exchange.getRequestURI().getPath())) {
                    sendResponse(exchange, 404, "Not Found");
                    return;
                }

                String sid = getCookieValue(exchange.getRequestHeaders(), "SID");
                boolean newSession = false;
                if (sid == null || sid.isEmpty() || !sid.matches("^[A-Fa-f0-9]{64}$") || !SESSIONS.containsKey(sid)) {
                    sid = generateSessionId();
                    // For demo purposes, populate session if new. In a real app, this would be set after authentication.
                    SESSIONS.put(sid, new SessionData("demo_user", "demo.user@example.com", "111-22-3333"));
                    newSession = true;
                }

                SessionData sd = SESSIONS.get(sid);
                String body;
                if (sd != null) {
                    body = renderInfoPage(sd.username, sd.email, sd.ssn);
                } else {
                    body = safeHtml("Session not found.");
                }

                Headers respHeaders = exchange.getResponseHeaders();
                setSecurityHeaders(respHeaders);
                if (newSession) {
                    // Set secure session cookie attributes
                    String cookie = "SID=" + sid + "; Path=/; HttpOnly; SameSite=Strict";
                    // 'Secure' is recommended for HTTPS; included here but will be ignored over plain HTTP by browsers
                    cookie += "; Secure";
                    respHeaders.add("Set-Cookie", cookie);
                }

                byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(bytes);
                }
            } finally {
                exchange.close();
            }
        }
    }

    // Core function: accepts inputs and returns the resulting HTML page.
    public static String renderInfoPage(String username, String email, String ssn) {
        String safeUser = safeHtml(username == null ? "" : username);
        String safeEmail = safeHtml(email == null ? "" : email);
        String last4 = last4Digits(ssn == null ? "" : ssn);
        String now = ZonedDateTime.now().format(DateTimeFormatter.RFC_1123_DATE_TIME);
        StringBuilder sb = new StringBuilder();
        sb.append("<!doctype html><html lang=\"en\"><head>")
          .append("<meta charset=\"utf-8\">")
          .append("<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'self'\">")
          .append("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">")
          .append("<title>Info</title>")
          .append("</head><body>")
          .append("<h1>Account Info</h1>")
          .append("<ul>")
          .append("<li>Username: ").append(safeUser).append("</li>")
          .append("<li>Email: ").append(safeEmail).append("</li>")
          .append("<li>SSN (last 4): ").append(last4).append("</li>")
          .append("</ul>")
          .append("<p>Generated at ").append(safeHtml(now)).append("</p>")
          .append("</body></html>");
        return sb.toString();
    }

    private static void setSecurityHeaders(Headers headers) {
        headers.add("Content-Type", "text/html; charset=utf-8");
        headers.add("X-Content-Type-Options", "nosniff");
        headers.add("X-Frame-Options", "DENY");
        headers.add("Referrer-Policy", "no-referrer");
        headers.add("Cache-Control", "no-store");
    }

    private static String getCookieValue(Headers headers, String name) {
        for (String cookieHeader : headers.getOrDefault("Cookie", Collections.emptyList())) {
            Map<String, String> cookies = parseCookieHeader(cookieHeader);
            if (cookies.containsKey(name)) {
                return cookies.get(name);
            }
        }
        return null;
    }

    private static Map<String, String> parseCookieHeader(String header) {
        Map<String, String> map = new HashMap<>();
        if (header == null) return map;
        String[] parts = header.split(";");
        for (String part : parts) {
            String[] kv = part.trim().split("=", 2);
            if (kv.length == 2) {
                map.put(kv[0].trim(), kv[1].trim());
            }
        }
        return map;
    }

    private static String generateSessionId() {
        byte[] bytes = new byte[32]; // 256-bit
        SECURE_RANDOM.nextBytes(bytes);
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) {
            sb.append(String.format(Locale.ROOT, "%02x", b));
        }
        return sb.toString();
    }

    private static String safeHtml(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#x27;"); break;
                case '/': out.append("&#x2F;"); break;
                default:
                    if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') {
                        // skip control characters
                    } else {
                        out.append(c);
                    }
            }
        }
        return out.toString();
    }

    private static String last4Digits(String ssn) {
        if (ssn == null) return "****";
        String digits = ssn.replaceAll("[^0-9]", "");
        if (digits.length() < 4) return "****";
        return digits.substring(digits.length() - 4);
    }
}