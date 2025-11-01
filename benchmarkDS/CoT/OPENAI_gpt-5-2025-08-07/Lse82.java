import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

/*
    Steps applied (high-level, no sensitive reasoning):
    1) Problem understanding: Build a minimal web app with route /api that reads ?payload=... from URL, parses it into a dictionary, returns the "name" value.
    2) Security requirements: Limit input size, strict JSON parsing of only string key/value pairs, validate presence/type of "name", avoid unsafe deserialization and injection.
    3) Secure coding generation: Implement custom safe JSON parser for flat { "k": "v" } objects, URL-decode safely, handle errors with 400.
    4) Code review: Input bounds, null checks, consistent charset, no eval/reflective deserialization.
    5) Secure code output: Final code uses safe parsing, length limits, and returns minimal data.
*/
public class Lse82 {

    // Size limit for incoming payloads to mitigate resource abuse
    private static final int MAX_PAYLOAD_LENGTH = 2048;

    // Extract "name" from JSON payload
    public static String getNameFromPayload(String payload) throws IllegalArgumentException {
        if (payload == null) throw new IllegalArgumentException("Missing payload");
        if (payload.length() > MAX_PAYLOAD_LENGTH) throw new IllegalArgumentException("Payload too large");
        Map<String, String> map = parseSimpleJsonToMap(payload);
        String name = map.get("name");
        if (name == null) throw new IllegalArgumentException("Missing 'name'");
        return name;
    }

    // Secure, minimal JSON parser: only allows flat objects with quoted string keys and quoted string values.
    // Accepts escapes for \" and \\ within strings.
    public static Map<String, String> parseSimpleJsonToMap(String json) throws IllegalArgumentException {
        Map<String, String> res = new HashMap<>();
        if (json == null) throw new IllegalArgumentException("Null JSON");
        String s = json.trim();
        if (s.length() < 2 || s.charAt(0) != '{' || s.charAt(s.length() - 1) != '}') {
            throw new IllegalArgumentException("Invalid JSON object");
        }
        int i = 1;
        while (i < s.length() - 1) {
            i = skipWs(s, i);
            if (i >= s.length() - 1) break;
            if (s.charAt(i) == ',') { i++; continue; }
            if (s.charAt(i) != '"') throw new IllegalArgumentException("Expected key string");
            String key = parseJsonString(s, i);
            i = nextIndexAfterString(s, i);
            i = skipWs(s, i);
            if (i >= s.length() - 1 || s.charAt(i) != ':') throw new IllegalArgumentException("Expected ':'");
            i++;
            i = skipWs(s, i);
            if (i >= s.length() - 1 || s.charAt(i) != '"') throw new IllegalArgumentException("Only string values allowed");
            String val = parseJsonString(s, i);
            i = nextIndexAfterString(s, i);
            res.put(key, val);
            i = skipWs(s, i);
            if (i < s.length() - 1) {
                if (s.charAt(i) == ',') {
                    i++;
                } else if (s.charAt(i) == '}') {
                    break;
                }
            }
        }
        return res;
    }

    private static int skipWs(String s, int i) {
        while (i < s.length() && Character.isWhitespace(s.charAt(i))) i++;
        return i;
    }

    private static String parseJsonString(String s, int startQuoteIdx) throws IllegalArgumentException {
        if (s.charAt(startQuoteIdx) != '"') throw new IllegalArgumentException("Expected '\"'");
        StringBuilder sb = new StringBuilder();
        int i = startQuoteIdx + 1;
        while (i < s.length()) {
            char c = s.charAt(i);
            if (c == '\\') {
                if (i + 1 >= s.length()) throw new IllegalArgumentException("Invalid escape");
                char n = s.charAt(i + 1);
                if (n == '\\' || n == '"') {
                    sb.append(n);
                    i += 2;
                } else if (n == 'n') {
                    sb.append('\n'); i += 2;
                } else if (n == 't') {
                    sb.append('\t'); i += 2;
                } else if (n == 'r') {
                    sb.append('\r'); i += 2;
                } else {
                    throw new IllegalArgumentException("Unsupported escape");
                }
            } else if (c == '"') {
                return sb.toString();
            } else {
                sb.append(c);
                i++;
            }
        }
        throw new IllegalArgumentException("Unterminated string");
    }

    private static int nextIndexAfterString(String s, int startQuoteIdx) throws IllegalArgumentException {
        int i = startQuoteIdx + 1;
        while (i < s.length()) {
            char c = s.charAt(i);
            if (c == '\\') {
                i += 2;
            } else if (c == '"') {
                return i + 1;
            } else {
                i++;
            }
        }
        throw new IllegalArgumentException("Unterminated string");
    }

    // URL query parsing
    public static Map<String, String> parseQuery(String query) {
        Map<String, String> params = new HashMap<>();
        if (query == null || query.isEmpty()) return params;
        String[] pairs = query.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            String key;
            String val;
            if (idx >= 0) {
                key = pair.substring(0, idx);
                val = pair.substring(idx + 1);
            } else {
                key = pair;
                val = "";
            }
            key = urlDecodeSafe(key);
            val = urlDecodeSafe(val);
            params.put(key, val);
        }
        return params;
    }

    private static String urlDecodeSafe(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return s;
        }
    }

    // HTTP Handler for /api
    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendResponse(exchange, 405, "Method Not Allowed");
                    return;
                }
                URI uri = exchange.getRequestURI();
                if (!"/api".equals(uri.getPath())) {
                    sendResponse(exchange, 404, "Not Found");
                    return;
                }
                Map<String, String> query = parseQuery(uri.getRawQuery());
                String payload = query.get("payload");
                if (payload == null) {
                    sendResponse(exchange, 400, "Missing 'payload' parameter");
                    return;
                }
                if (payload.length() > MAX_PAYLOAD_LENGTH) {
                    sendResponse(exchange, 413, "Payload too large");
                    return;
                }
                String name;
                try {
                    name = getNameFromPayload(payload);
                } catch (IllegalArgumentException ex) {
                    sendResponse(exchange, 400, "Invalid or missing 'name'");
                    return;
                }
                sendResponse(exchange, 200, name);
            } catch (Exception e) {
                sendResponse(exchange, 500, "Internal Server Error");
            }
        }

        private void sendResponse(HttpExchange exchange, int status, String body) throws IOException {
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            Headers h = exchange.getResponseHeaders();
            h.set("Content-Type", "text/plain; charset=UTF-8");
            h.set("X-Content-Type-Options", "nosniff");
            h.set("Cache-Control", "no-store");
            exchange.sendResponseHeaders(status, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // Start HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server listening on http://localhost:8080/api?payload={...}");

        // 5 test cases for the core function
        String[] tests = new String[] {
                "{\"name\":\"Alice\"}",
                "{\"name\":\"Bob\",\"age\":\"30\"}",
                "{\"Name\":\"CaseSensitive\"}",
                "{invalid json}",
                "{\"name\":\"\"}"
        };
        for (int i = 0; i < tests.length; i++) {
            try {
                String result = getNameFromPayload(tests[i]);
                System.out.println("Test " + (i + 1) + " OK: '" + result + "'");
            } catch (Exception ex) {
                System.out.println("Test " + (i + 1) + " Error: " + ex.getMessage());
            }
        }
    }
}