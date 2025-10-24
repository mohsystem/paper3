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
import java.util.Map;

public class Task38 {

    // Generate the HTML form page with a given title
    public static String generateFormPage(String title) {
        String safeTitle = htmlEscape(title);
        return "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>" + safeTitle + "</title></head>"
                + "<body>"
                + "<h1>" + safeTitle + "</h1>"
                + "<form method=\"POST\" action=\"/echo\">"
                + "<label>Enter a message: <input type=\"text\" name=\"message\" /></label>"
                + "<button type=\"submit\">Submit</button>"
                + "</form>"
                + "</body></html>";
    }

    // Generate the HTML echo page for a given message
    public static String generateEchoPage(String message) {
        String safeMsg = htmlEscape(message == null ? "" : message);
        return "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Echo Result</title></head>"
                + "<body>"
                + "<h1>Echo Result</h1>"
                + "<p>You entered:</p>"
                + "<div style=\"padding:8px;border:1px solid #ccc;background:#f9f9f9;white-space:pre-wrap;\">" + safeMsg + "</div>"
                + "<p><a href=\"/\">Go back</a></p>"
                + "</body></html>";
    }

    // HTML escape helper
    public static String htmlEscape(String s) {
        StringBuilder out = new StringBuilder();
        for (char c : s.toCharArray()) {
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

    // Parse application/x-www-form-urlencoded body to a Map
    public static Map<String, String> parseFormBody(String body) {
        Map<String, String> map = new LinkedHashMap<>();
        if (body == null || body.isEmpty()) return map;
        String[] pairs = body.split("&");
        for (String p : pairs) {
            String[] kv = p.split("=", 2);
            String key = urlDecode(kv[0]);
            String val = kv.length > 1 ? urlDecode(kv[1]) : "";
            map.put(key, val);
        }
        return map;
    }

    // URL decode helper for form values
    public static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return s;
        }
    }

    // Read request body into a UTF-8 string
    public static String readRequestBody(HttpExchange ex) throws IOException {
        InputStream is = ex.getRequestBody();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[4096];
        int r;
        while ((r = is.read(buf)) != -1) {
            baos.write(buf, 0, r);
        }
        return baos.toString(StandardCharsets.UTF_8);
    }

    // Send HTTP response with given status and body (UTF-8)
    public static void sendResponse(HttpExchange ex, int status, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        Headers h = ex.getResponseHeaders();
        h.set("Content-Type", "text/html; charset=utf-8");
        ex.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(bytes);
        }
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            String method = ex.getRequestMethod();
            if ("GET".equalsIgnoreCase(method)) {
                sendResponse(ex, 200, generateFormPage("Echo App"));
            } else {
                sendResponse(ex, 405, "<h1>405 Method Not Allowed</h1>");
            }
        }
    }

    static class EchoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            String method = ex.getRequestMethod();
            if ("POST".equalsIgnoreCase(method)) {
                String body = readRequestBody(ex);
                Map<String, String> form = parseFormBody(body);
                String message = form.getOrDefault("message", "");
                sendResponse(ex, 200, generateEchoPage(message));
            } else if ("GET".equalsIgnoreCase(method)) {
                // Optional: allow GET with query ?message=...
                String query = ex.getRequestURI().getRawQuery();
                Map<String, String> qs = parseFormBody(query == null ? "" : query);
                String message = qs.getOrDefault("message", "");
                sendResponse(ex, 200, generateEchoPage(message));
            } else {
                sendResponse(ex, 405, "<h1>405 Method Not Allowed</h1>");
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 simple test cases for the generation functions
        String[] tests = new String[] {
                "Hello, World!",
                "<b>bold</b> & \"quotes\"",
                "100% encoded & safe",
                "Line1\nLine2\nLine3",
                "Emoji: 😊 漢字"
        };
        for (int i = 0; i < tests.length; i++) {
            String html = generateEchoPage(tests[i]);
            System.out.println("Test " + (i + 1) + " generated HTML length: " + html.length());
        }

        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/echo", new EchoHandler());
        server.setExecutor(null);
        System.out.println("Java server running at http://localhost:" + port + " (Press Ctrl+C to stop)");
        server.start();
    }
}