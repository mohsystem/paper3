import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task38 {

    // Security: in-memory CSRF tokens per session id (sid)
    private static final Map<String, String> TOKENS = new ConcurrentHashMap<>();
    private static final SecureRandom RNG = new SecureRandom();
    private static final int MAX_BODY = 4096;
    private static final int MAX_MSG = 200;
    private static final String ALLOWED = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,;:!?@#()_-'\"]";

    public static void main(String[] args) throws Exception {
        // 5 test cases for sanitize/validate
        System.out.println("Test1: " + testProcess("<script>alert(1)</script>")); // expect escaped
        System.out.println("Test2: " + testProcess("Hello, World!")); // ok
        System.out.println("Test3: " + testProcess("This_is-OK() #1!?")); // ok
        System.out.println("Test4: " + testProcess(generateLongString(500))); // INVALID
        System.out.println("Test5: " + testProcess("Emoji not allowed: 🙂")); // INVALID

        // Start secure minimal web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/echo", new EchoHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server running on http://localhost:8080/");
    }

    private static String testProcess(String input) {
        String v = validateMessage(input);
        return v == null ? "INVALID" : v;
    }

    private static String generateLongString(int n) {
        char[] c = new char[n];
        Arrays.fill(c, 'A');
        return new String(c);
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            if (!ex.getRequestMethod().equalsIgnoreCase("GET")) {
                sendText(ex, 405, "Method Not Allowed");
                return;
            }
            Headers req = ex.getRequestHeaders();
            String sid = getCookie(req, "sid");
            if (sid == null || sid.length() < 16) {
                sid = genToken(16);
            }
            String csrf = genToken(32);
            TOKENS.put(sid, csrf);

            Headers res = ex.getResponseHeaders();
            setSecurityHeaders(res);
            res.add("Set-Cookie", "sid=" + sid + "; HttpOnly; Path=/; SameSite=Strict");

            String body = "<!doctype html><html><head><meta charset=\"utf-8\">" +
                    "<title>Echo</title>" +
                    cspMeta() +
                    "</head><body>" +
                    "<h1>Secure Echo</h1>" +
                    "<form method=\"POST\" action=\"/echo\">" +
                    "<label>Message: <input type=\"text\" name=\"message\" maxlength=\"" + MAX_MSG + "\"></label>" +
                    "<input type=\"hidden\" name=\"csrf\" value=\"" + csrf + "\">" +
                    "<button type=\"submit\">Send</button>" +
                    "</form>" +
                    "</body></html>";
            sendHtml(ex, 200, body);
        }
    }

    static class EchoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            if (!ex.getRequestMethod().equalsIgnoreCase("POST")) {
                sendText(ex, 405, "Method Not Allowed");
                return;
            }
            Headers req = ex.getRequestHeaders();
            String contentType = Optional.ofNullable(req.getFirst("Content-Type")).orElse("");
            if (!contentType.toLowerCase(Locale.ROOT).startsWith("application/x-www-form-urlencoded")) {
                sendText(ex, 415, "Unsupported Media Type");
                return;
            }

            byte[] bodyBytes = readBody(ex.getRequestBody(), MAX_BODY);
            if (bodyBytes == null) {
                sendText(ex, 413, "Payload Too Large");
                return;
            }
            String bodyStr = new String(bodyBytes, StandardCharsets.UTF_8);
            Map<String, String> form = parseUrlEncoded(bodyStr);

            String sid = getCookie(req, "sid");
            String expectedCsrf = sid == null ? null : TOKENS.get(sid);
            String gotCsrf = form.getOrDefault("csrf", "");
            if (sid == null || expectedCsrf == null || !constantTimeEq(expectedCsrf, gotCsrf)) {
                sendHtml(ex, 400, htmlPage("Invalid CSRF token."));
                return;
            }

            String raw = form.getOrDefault("message", "");
            String safe = validateMessage(raw);
            if (safe == null) {
                sendHtml(ex, 400, htmlPage("Invalid input. Only basic punctuation and letters/numbers allowed, max " + MAX_MSG + " chars."));
                return;
            }

            Headers res = ex.getResponseHeaders();
            setSecurityHeaders(res);
            res.add("Set-Cookie", "sid=" + sid + "; HttpOnly; Path=/; SameSite=Strict");

            String response = "<!doctype html><html><head><meta charset=\"utf-8\">" +
                    "<title>Echo</title>" + cspMeta() +
                    "</head><body>" +
                    "<h1>Echo result</h1>" +
                    "<p>You said: <strong>" + htmlEscape(safe) + "</strong></p>" +
                    "<p><a href=\"/\">Back</a></p>" +
                    "</body></html>";
            sendHtml(ex, 200, response);
        }
    }

    private static void setSecurityHeaders(Headers res) {
        res.add("Content-Type", "text/html; charset=utf-8");
        res.add("X-Content-Type-Options", "nosniff");
        res.add("Referrer-Policy", "no-referrer");
        res.add("X-Frame-Options", "DENY");
        res.add("Content-Security-Policy", "default-src 'none'; style-src 'self' 'unsafe-inline'; script-src 'none'; img-src 'self'; base-uri 'none'; form-action 'self'");
    }

    private static String cspMeta() {
        return "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; style-src 'self' 'unsafe-inline'; script-src 'none'; img-src 'self'; base-uri 'none'; form-action 'self'\">";
    }

    private static String htmlPage(String msg) {
        return "<!doctype html><html><head><meta charset=\"utf-8\">" + cspMeta() +
                "<title>Echo</title></head><body><p>" + htmlEscape(msg) +
                "</p><p><a href=\"/\">Back</a></p></body></html>";
    }

    private static void sendText(HttpExchange ex, int code, String text) throws IOException {
        Headers res = ex.getResponseHeaders();
        setSecurityHeaders(res);
        res.set("Content-Type", "text/plain; charset=utf-8");
        byte[] b = text.getBytes(StandardCharsets.UTF_8);
        ex.sendResponseHeaders(code, b.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(b);
        }
    }

    private static void sendHtml(HttpExchange ex, int code, String html) throws IOException {
        Headers res = ex.getResponseHeaders();
        setSecurityHeaders(res);
        byte[] b = html.getBytes(StandardCharsets.UTF_8);
        ex.sendResponseHeaders(code, b.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(b);
        }
    }

    private static byte[] readBody(InputStream is, int max) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[1024];
        int total = 0;
        int r;
        while ((r = is.read(buf)) != -1) {
            total += r;
            if (total > max) return null;
            baos.write(buf, 0, r);
        }
        return baos.toByteArray();
    }

    private static String htmlEscape(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                case '/': sb.append("&#x2F;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    private static boolean constantTimeEq(String a, String b) {
        if (a == null || b == null) return false;
        byte[] x = a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b.getBytes(StandardCharsets.UTF_8);
        if (x.length != y.length) return false;
        int r = 0;
        for (int i = 0; i < x.length; i++) r |= x[i] ^ y[i];
        return r == 0;
    }

    private static String validateMessage(String raw) {
        if (raw == null) return null;
        if (raw.length() == 0 || raw.length() > MAX_MSG) return null;
        for (int i = 0; i < raw.length(); i++) {
            char c = raw.charAt(i);
            if (c == '\r' || c == '\n' || c == '\t') return null;
            if (ALLOWED.indexOf(c) < 0) return null;
        }
        return raw;
    }

    private static Map<String, String> parseUrlEncoded(String body) throws UnsupportedEncodingException {
        Map<String, String> map = new HashMap<>();
        for (String pair : body.split("&")) {
            if (pair.isEmpty()) continue;
            String[] kv = pair.split("=", 2);
            String k = URLDecoder.decode(kv[0], "UTF-8");
            String v = kv.length > 1 ? URLDecoder.decode(kv[1], "UTF-8") : "";
            map.put(k, v);
        }
        return map;
    }

    private static String genToken(int bytes) {
        byte[] b = new byte[bytes];
        RNG.nextBytes(b);
        StringBuilder sb = new StringBuilder(bytes * 2);
        for (byte x : b) sb.append(String.format("%02x", x));
        return sb.toString();
    }

    private static String getCookie(Headers headers, String name) {
        List<String> cookies = headers.get("Cookie");
        if (cookies == null) return null;
        for (String c : cookies) {
            String[] parts = c.split(";");
            for (String p : parts) {
                String[] nv = p.trim().split("=", 2);
                if (nv.length == 2 && nv[0].equals(name)) return nv[1];
            }
        }
        return null;
    }
}