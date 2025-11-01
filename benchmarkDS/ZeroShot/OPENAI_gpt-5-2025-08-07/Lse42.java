import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;

import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;

import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;

public class Lse42 {
    private static final Map<String, UserSession> SESSIONS = new ConcurrentHashMap<>();
    private static final SecureRandom RNG = new SecureRandom();

    public static final class UserSession {
        public final String username;
        public final String email;
        public final String ssn;

        public UserSession(String username, String email, String ssn) {
            this.username = username;
            this.email = email;
            this.ssn = ssn;
        }
    }

    // info() function: accepts a session and returns the response string
    public static String info(UserSession session) {
        if (session == null) {
            return "Unauthorized";
        }
        String u = sanitizeUserOrEmail(session.username, 100);
        String e = sanitizeUserOrEmail(session.email, 254);
        String s = sanitizeSSN(session.ssn, 11);
        return "username=" + u + "; email=" + e + "; ssn=" + s;
    }

    private static String sanitizeUserOrEmail(String v, int maxLen) {
        if (v == null) return "";
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (char c : v.toCharArray()) {
            if (count >= maxLen) break;
            // Allow alnum, space and typical email/name symbols; exclude CR/LF and control chars
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '@' || c == '.' || c == '-' || c == '_' || c == '+' || c == ' ') {
                sb.append(c);
                count++;
            }
        }
        return sb.toString();
    }

    private static String sanitizeSSN(String v, int maxLen) {
        if (v == null) return "";
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (char c : v.toCharArray()) {
            if (count >= maxLen) break;
            if ((c >= '0' && c <= '9') || c == '-') {
                sb.append(c);
                count++;
            }
        }
        return sb.toString();
    }

    private static String genSessionId() {
        byte[] b = new byte[32];
        RNG.nextBytes(b);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(b);
    }

    private static String parseSidFromCookies(List<String> cookieHeaders) {
        if (cookieHeaders == null) return null;
        for (String header : cookieHeaders) {
            String[] parts = header.split(";");
            for (String p : parts) {
                String kv = p.trim();
                if (kv.startsWith("SID=")) {
                    return kv.substring(4);
                }
            }
        }
        return null;
    }

    private static void writeResponse(HttpExchange ex, int status, String body) throws IOException {
        Headers h = ex.getResponseHeaders();
        h.set("Content-Type", "text/plain; charset=utf-8");
        h.set("X-Content-Type-Options", "nosniff");
        h.set("Content-Security-Policy", "default-src 'none'");
        h.set("Cache-Control", "no-store");
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        ex.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(bytes);
        }
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(ex.getRequestMethod())) {
                    writeResponse(ex, 405, "Method Not Allowed");
                    return;
                }
                String path = ex.getRequestURI().getPath();
                if (!"/info".equals(path)) {
                    writeResponse(ex, 404, "Not Found");
                    return;
                }
                String sid = parseSidFromCookies(ex.getRequestHeaders().get("Cookie"));
                UserSession session = sid == null ? null : SESSIONS.get(sid);
                if (session == null) {
                    writeResponse(ex, 401, "Unauthorized");
                    return;
                }
                String out = info(session);
                writeResponse(ex, 200, out);
            } finally {
                ex.close();
            }
        }
    }

    private static String httpGet(String url, String cookie) throws IOException {
        HttpURLConnection conn = (HttpURLConnection) new URL(url).openConnection();
        conn.setRequestMethod("GET");
        conn.setInstanceFollowRedirects(false);
        if (cookie != null) {
            conn.setRequestProperty("Cookie", cookie);
        }
        conn.setRequestProperty("Accept", "text/plain");
        int code = conn.getResponseCode();
        BufferedReader br = new BufferedReader(new InputStreamReader(
                (code >= 200 && code < 400) ? conn.getInputStream() : conn.getErrorStream(),
                StandardCharsets.UTF_8));
        StringBuilder sb = new StringBuilder();
        String line;
        while ((line = br.readLine()) != null) {
            sb.append(line);
        }
        br.close();
        return code + ":" + sb.toString();
    }

    private static String maskSSNInBody(String body) {
        // Mask SSN patterns like 123-45-6789 to ***-**-6789 in printed test outputs
        return body.replaceAll("\\b(\\d{3})-(\\d{2})-(\\d{4})\\b", "***-**-$3");
    }

    public static void main(String[] args) throws Exception {
        // Start server
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 18080), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();

        // Prepare 4 valid sessions
        String sid1 = genSessionId();
        String sid2 = genSessionId();
        String sid3 = genSessionId();
        String sid4 = genSessionId();
        SESSIONS.put(sid1, new UserSession("alice", "alice@example.com", "123-45-6789"));
        SESSIONS.put(sid2, new UserSession("bob", "bob.smith+dev@example.org", "000-12-3456"));
        SESSIONS.put(sid3, new UserSession("charlie_ze", "charlie.ze@example.net", "999-99-9999"));
        SESSIONS.put(sid4, new UserSession("dana-j", "dana-j@example.co", "321-54-9876"));

        // Allow server to bind
        Thread.sleep(200);

        // Five test cases
        List<String> tests = Arrays.asList(
                "SID=" + sid1,
                "SID=" + sid2,
                "SID=" + sid3,
                "SID=" + sid4,
                "SID=invalid_session_token"
        );

        for (int i = 0; i < tests.size(); i++) {
            String cookie = tests.get(i);
            String res = httpGet("http://127.0.0.1:18080/info", cookie);
            // Do not log full SSN in tests
            System.out.println("Test " + (i + 1) + " -> " + maskSSNInBody(res));
        }

        server.stop(0);
    }
}