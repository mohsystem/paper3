import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Lse42 {

    private static final Map<String, Map<String, String>> SESSIONS = new ConcurrentHashMap<>();
    private static final SecureRandom RNG = new SecureRandom();

    public static String info(Map<String, String> session) {
        String username = sanitizeUser(session.getOrDefault("username", "unknown"));
        String email = sanitizeEmail(session.getOrDefault("email", "unknown@example.com"));
        String ssnMasked = maskSSN(session.getOrDefault("ssn", "000-00-0000"));
        return "username: " + username + "; email: " + email + "; ssn: " + ssnMasked;
    }

    private static String sanitizeUser(String v) {
        if (v == null) return "unknown";
        StringBuilder sb = new StringBuilder();
        for (char c : v.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == '-' || c == '_' || c == '.') {
                sb.append(c);
            }
        }
        return sb.length() == 0 ? "unknown" : sb.toString();
    }

    private static String sanitizeEmail(String v) {
        if (v == null) return "unknown@example.com";
        StringBuilder sb = new StringBuilder();
        for (char c : v.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == '-' || c == '_' || c == '.' || c == '@') {
                sb.append(c);
            }
        }
        String out = sb.toString();
        if (!out.contains("@")) {
            return "unknown@example.com";
        }
        return out;
    }

    private static String maskSSN(String ssn) {
        if (ssn == null) return "*********";
        int totalDigits = 0;
        for (int i = 0; i < ssn.length(); i++) {
            if (Character.isDigit(ssn.charAt(i))) totalDigits++;
        }
        int keep = Math.min(4, totalDigits);
        int seen = 0;
        StringBuilder rev = new StringBuilder();
        for (int i = ssn.length() - 1; i >= 0; --i) {
            char c = ssn.charAt(i);
            if (Character.isDigit(c)) {
                seen++;
                rev.append(seen <= keep ? c : '*');
            } else {
                rev.append(c);
            }
        }
        return rev.reverse().toString();
    }

    private static String generateSID() {
        byte[] b = new byte[16];
        RNG.nextBytes(b);
        StringBuilder sb = new StringBuilder();
        for (byte x : b) {
            sb.append(String.format("%02x", x));
        }
        return sb.toString();
    }

    private static String getCookieSID(Headers headers) {
        List<String> cookies = headers.get("Cookie");
        if (cookies == null) return null;
        for (String line : cookies) {
            String[] parts = line.split(";");
            for (String p : parts) {
                String[] kv = p.trim().split("=", 2);
                if (kv.length == 2 && kv[0].equals("SID")) {
                    return kv[1];
                }
            }
        }
        return null;
    }

    private static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1);
                return;
            }
            String path = exchange.getRequestURI().getPath();
            if (!"/info".equals(path)) {
                exchange.sendResponseHeaders(404, -1);
                return;
            }

            Headers req = exchange.getRequestHeaders();
            Headers resp = exchange.getResponseHeaders();

            String sid = getCookieSID(req);
            boolean setCookie = false;
            if (sid == null || !SESSIONS.containsKey(sid)) {
                sid = generateSID();
                Map<String, String> sess = new HashMap<>();
                sess.put("username", "demo_user");
                sess.put("email", "demo_user@example.com");
                sess.put("ssn", "123-45-6789");
                SESSIONS.put(sid, sess);
                setCookie = true;
            }

            Map<String, String> session = SESSIONS.getOrDefault(sid, Collections.emptyMap());
            String body = info(session);

            resp.set("Content-Type", "text/plain; charset=utf-8");
            resp.set("X-Content-Type-Options", "nosniff");
            resp.set("Cache-Control", "no-store");
            if (setCookie) {
                resp.add("Set-Cookie", "SID=" + sid + "; HttpOnly; SameSite=Lax; Path=/");
            }

            byte[] data = body.getBytes("UTF-8");
            exchange.sendResponseHeaders(200, data.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(data);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for the info() function
        Map<String, String> t1 = new HashMap<>();
        t1.put("username", "alice");
        t1.put("email", "alice@example.com");
        t1.put("ssn", "123-45-6789");
        System.out.println(info(t1));

        Map<String, String> t2 = new HashMap<>();
        t2.put("username", "bob_123");
        t2.put("email", "bob-123@domain.co.uk");
        t2.put("ssn", "987654321");
        System.out.println(info(t2));

        Map<String, String> t3 = new HashMap<>();
        t3.put("username", "charlie!");
        t3.put("email", "charlie_at_example.com");
        t3.put("ssn", "111-22-3333");
        System.out.println(info(t3));

        Map<String, String> t4 = new HashMap<>();
        t4.put("username", "dora.d");
        t4.put("email", "dora.d@example.com");
        t4.put("ssn", "000000000");
        System.out.println(info(t4));

        Map<String, String> t5 = new HashMap<>();
        // missing fields to exercise defaults
        System.out.println(info(t5));

        // Start simple HTTP server at http://localhost:8080/info
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        server.start();
    }
}