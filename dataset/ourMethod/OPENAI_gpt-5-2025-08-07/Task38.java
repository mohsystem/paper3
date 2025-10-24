import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class Task38 {
    private static final int PORT = 8080;
    private static final int MAX_HEADER_SIZE = 8192;
    private static final int MAX_BODY_SIZE = 4096;
    private static final int MAX_MESSAGE_LEN = 200;

    public static void main(String[] args) throws Exception {
        // Basic tests for sanitizer and escaper
        runSelfTests();

        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/submit", new SubmitHandler());
        server.setExecutor(null);
        System.out.println("Server started on http://localhost:" + PORT + "/");
        server.start();
    }

    private static void runSelfTests() {
        String[] tests = new String[]{
                "<script>alert('x')</script>",
                "Hello & Goodbye",
                "   Trim me please   ",
                "A".repeat(500),
                "Control\u0001Chars\u0002Removed"
        };
        for (int i = 0; i < tests.length; i++) {
            String sanitized = sanitizeInput(tests[i], MAX_MESSAGE_LEN);
            String escaped = escapeHtml(sanitized);
            System.out.println("Test " + (i + 1) + ": input=[" + tests[i] + "] sanitized=[" + sanitized + "] escaped=[" + escaped + "]");
        }
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String method = exchange.getRequestMethod();
            if (!"GET".equalsIgnoreCase(method)) {
                sendPlain(exchange, 405, "Method Not Allowed");
                return;
            }
            String html = renderPage(null);
            sendHtml(exchange, 200, html);
        }
    }

    static class SubmitHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String method = exchange.getRequestMethod();
            if (!"POST".equalsIgnoreCase(method)) {
                sendPlain(exchange, 405, "Method Not Allowed");
                return;
            }
            Headers headers = exchange.getRequestHeaders();
            String ct = firstHeader(headers, "Content-Type");
            if (ct == null || !ct.toLowerCase().startsWith("application/x-www-form-urlencoded")) {
                sendPlain(exchange, 400, "Bad Request: Unsupported Content-Type");
                return;
            }
            int contentLength = parseContentLength(firstHeader(headers, "Content-Length"));
            if (contentLength < 0 || contentLength > MAX_BODY_SIZE) {
                sendPlain(exchange, 413, "Payload Too Large");
                return;
            }
            byte[] body = readRequestBody(exchange.getRequestBody(), contentLength, MAX_BODY_SIZE);
            if (body == null) {
                sendPlain(exchange, 400, "Bad Request: Invalid body");
                return;
            }
            String form = new String(body, StandardCharsets.UTF_8);
            Map<String, String> params = parseUrlEncoded(form);
            String message = params.getOrDefault("message", "");
            String sanitized = sanitizeInput(message, MAX_MESSAGE_LEN);
            String escaped = escapeHtml(sanitized);
            String html = renderPage("You submitted: <strong>" + escaped + "</strong>");
            sendHtml(exchange, 200, html);
        }
    }

    private static String renderPage(String resultHtml) {
        StringBuilder sb = new StringBuilder();
        sb.append("<!DOCTYPE html><html lang=\"en\"><head>")
          .append("<meta charset=\"UTF-8\">")
          .append("<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">")
          .append("<meta http-equiv=\"Referrer-Policy\" content=\"no-referrer\">")
          .append("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">")
          .append("<title>Echo App</title>")
          .append("<style>")
          .append("body{font-family:sans-serif;margin:2rem;}")
          .append("input[type=text]{width:300px;max-width:90%;padding:.4rem;}")
          .append("button{padding:.4rem .8rem;margin-left:.5rem;}")
          .append(".result{margin-top:1rem;padding:.8rem;background:#f4f6f8;border:1px solid #dfe3e8;border-radius:6px;}")
          .append("</style></head><body>")
          .append("<h1>Echo App</h1>")
          .append("<form method=\"post\" action=\"/submit\" autocomplete=\"off\">")
          .append("<label for=\"message\">Message (max 200 chars):</label><br>")
          .append("<input id=\"message\" name=\"message\" type=\"text\" maxlength=\"200\" required>")
          .append("<button type=\"submit\">Send</button>")
          .append("</form>");
        if (resultHtml != null) {
            sb.append("<div class=\"result\">").append(resultHtml).append("</div>");
        }
        sb.append("</body></html>");
        return sb.toString();
    }

    private static String firstHeader(Headers headers, String name) {
        List<String> vals = headers.get(name);
        if (vals == null || vals.isEmpty()) return null;
        return vals.get(0);
    }

    private static void sendHtml(HttpExchange ex, int status, String html) throws IOException {
        byte[] bytes = html.getBytes(StandardCharsets.UTF_8);
        Headers h = ex.getResponseHeaders();
        h.set("Content-Type", "text/html; charset=UTF-8");
        h.set("X-Content-Type-Options", "nosniff");
        h.set("Content-Security-Policy", "default-src 'none'; style-src 'self' 'unsafe-inline'; img-src 'self'; form-action 'self'");
        ex.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static void sendPlain(HttpExchange ex, int status, String msg) throws IOException {
        byte[] bytes = msg.getBytes(StandardCharsets.UTF_8);
        Headers h = ex.getResponseHeaders();
        h.set("Content-Type", "text/plain; charset=UTF-8");
        h.set("X-Content-Type-Options", "nosniff");
        ex.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static int parseContentLength(String s) {
        if (s == null) return -1;
        try {
            int v = Integer.parseInt(s.trim());
            if (v < 0) return -1;
            return v;
        } catch (NumberFormatException e) {
            return -1;
        }
    }

    private static byte[] readRequestBody(InputStream is, int toRead, int max) throws IOException {
        if (toRead < 0 || toRead > max) return null;
        ByteArrayOutputStream bos = new ByteArrayOutputStream(Math.min(toRead, max));
        byte[] buf = new byte[1024];
        int remaining = toRead;
        while (remaining > 0) {
            int r = is.read(buf, 0, Math.min(buf.length, remaining));
            if (r == -1) break;
            bos.write(buf, 0, r);
            remaining -= r;
        }
        if (bos.size() != toRead) {
            return null;
        }
        return bos.toByteArray();
    }

    private static Map<String, String> parseUrlEncoded(String form) {
        Map<String, String> map = new LinkedHashMap<>();
        if (form == null || form.isEmpty()) return map;
        String[] pairs = form.split("&");
        for (String p : pairs) {
            String[] kv = p.split("=", 2);
            String k = urlDecodeSafe(kv[0]);
            String v = kv.length > 1 ? urlDecodeSafe(kv[1]) : "";
            if (k != null) {
                map.put(k, v == null ? "" : v);
            }
        }
        return map;
    }

    private static String urlDecodeSafe(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return null;
        }
    }

    private static String escapeHtml(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    private static String sanitizeInput(String input, int maxLen) {
        if (input == null) return "";
        String trimmed = input.trim();
        StringBuilder sb = new StringBuilder(Math.min(trimmed.length(), maxLen));
        for (int i = 0; i < trimmed.length() && sb.length() < maxLen; i++) {
            char c = trimmed.charAt(i);
            if (c == '\n' || c == '\r' || c == '\t' || !Character.isISOControl(c)) {
                // allow non-control or permitted whitespace
                if (Character.isISOControl(c) && !(c == '\n' || c == '\r' || c == '\t')) continue;
                sb.append(c);
            }
        }
        return sb.toString();
    }
}