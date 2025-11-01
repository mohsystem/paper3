import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse82 {

    // Extracts the "name" field from a JSON object payload. Returns null if not present or invalid.
    public static String extractNameFromPayload(String payload) {
        if (payload == null) return null;
        if (payload.length() > 4096) return null; // size limit for safety
        String s = payload.trim();
        if (!s.startsWith("{") || !s.endsWith("}")) {
            return null;
        }
        // Find "name": "value" using a conservative scan and handle JSON escapes
        int keyIdx = indexOfJsonStringKey(s, "name");
        if (keyIdx < 0) return null;
        int colon = skipWhitespaceForward(s, keyIdx);
        if (colon >= s.length() || s.charAt(colon) != ':') return null;
        colon++;
        colon = skipWhitespaceForward(s, colon);
        if (colon >= s.length() || s.charAt(colon) != '"') return null;

        // Parse JSON string value
        ParseResult pr = parseJsonString(s, colon);
        if (!pr.ok) return null;
        return pr.value;
    }

    // Starts a simple HTTP server on localhost:8080 with /api route that reads ?payload=...
    public static void startServer() throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/api", Lse82::handleApi);
        server.setExecutor(Executors.newFixedThreadPool(2));
        server.start();
    }

    private static void handleApi(HttpExchange exchange) throws IOException {
        try {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                exchange.getResponseHeaders().add("Allow", "GET");
                exchange.sendResponseHeaders(405, -1);
                return;
            }
            URI uri = exchange.getRequestURI();
            String query = uri.getRawQuery();
            Map<String, String> params = parseQuery(query);
            String rawPayload = params.get("payload");
            if (rawPayload == null) {
                sendText(exchange, 400, "Missing payload parameter");
                return;
            }
            if (rawPayload.length() > 16384) { // limit raw query value
                sendText(exchange, 413, "Payload too large");
                return;
            }
            String decoded = safeUrlDecode(rawPayload);
            if (decoded == null) {
                sendText(exchange, 400, "Invalid URL encoding");
                return;
            }
            String name = extractNameFromPayload(decoded);
            if (name == null) {
                sendText(exchange, 400, "Invalid payload or missing name");
                return;
            }
            sendText(exchange, 200, name);
        } catch (Exception e) {
            sendText(exchange, 500, "Internal Server Error");
        }
    }

    private static void sendText(HttpExchange exchange, int status, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new LinkedHashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] parts = query.split("&");
        for (String part : parts) {
            int idx = part.indexOf('=');
            String k = idx >= 0 ? part.substring(0, idx) : part;
            String v = idx >= 0 ? part.substring(idx + 1) : "";
            String dk = safeUrlDecode(k);
            String dv = safeUrlDecode(v);
            if (dk != null && dv != null) {
                map.put(dk, dv);
            }
        }
        return map;
    }

    private static String safeUrlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return null;
        }
    }

    private static int skipWhitespaceForward(String s, int i) {
        int n = s.length();
        int j = i;
        while (j < n) {
            char c = s.charAt(j);
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                j++;
            } else {
                break;
            }
        }
        return j;
    }

    private static int indexOfJsonStringKey(String s, String key) {
        // Looks for "key" followed by optional spaces and a colon, respecting escapes
        int n = s.length();
        int i = 0;
        while (i < n) {
            int quote = s.indexOf('"', i);
            if (quote < 0) return -1;
            ParseResult pr = parseJsonString(s, quote);
            if (!pr.ok) return -1;
            if (key.equals(pr.value)) {
                int after = pr.nextIndex;
                after = skipWhitespaceForward(s, after);
                if (after < n && s.charAt(after) == ':') {
                    return after;
                } else {
                    i = pr.nextIndex;
                    continue;
                }
            } else {
                i = pr.nextIndex;
            }
        }
        return -1;
    }

    private static class ParseResult {
        boolean ok;
        String value;
        int nextIndex; // index after the closing quote
    }

    private static ParseResult parseJsonString(String s, int startQuoteIdx) {
        ParseResult pr = new ParseResult();
        int n = s.length();
        if (startQuoteIdx >= n || s.charAt(startQuoteIdx) != '"') {
            pr.ok = false;
            return pr;
        }
        StringBuilder sb = new StringBuilder();
        int i = startQuoteIdx + 1;
        while (i < n) {
            char c = s.charAt(i);
            if (c == '"') {
                pr.ok = true;
                pr.value = sb.toString();
                pr.nextIndex = i + 1;
                return pr;
            } else if (c == '\\') {
                if (i + 1 >= n) {
                    pr.ok = false;
                    return pr;
                }
                char esc = s.charAt(++i);
                switch (esc) {
                    case '"': sb.append('"'); break;
                    case '\\': sb.append('\\'); break;
                    case '/': sb.append('/'); break;
                    case 'b': sb.append('\b'); break;
                    case 'f': sb.append('\f'); break;
                    case 'n': sb.append('\n'); break;
                    case 'r': sb.append('\r'); break;
                    case 't': sb.append('\t'); break;
                    case 'u':
                        if (i + 4 >= n) { pr.ok = false; return pr; }
                        String hex = s.substring(i + 1, i + 5);
                        if (!hex.matches("[0-9a-fA-F]{4}")) { pr.ok = false; return pr; }
                        int codePoint = Integer.parseInt(hex, 16);
                        sb.append((char) codePoint);
                        i += 4;
                        break;
                    default:
                        pr.ok = false;
                        return pr;
                }
                i++;
            } else {
                // Control characters are not allowed unescaped in JSON strings
                if (c >= 0 && c <= 0x1F) { pr.ok = false; return pr; }
                sb.append(c);
                i++;
            }
        }
        pr.ok = false;
        return pr;
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for extractNameFromPayload
        String[] tests = new String[] {
            "{\"name\":\"Alice\"}",
            "{\"age\":30}",
            "not json",
            "{\"name\":\"A\\u004c\\u0069\\u0063\\u0065\"}",
            "{\"name\":\"Bob \\\"The Builder\\\"\"}"
        };
        for (String t : tests) {
            String result = extractNameFromPayload(t);
            System.out.println(result == null ? "(null)" : result);
        }
        // Start the server
        startServer();
        // Keep main alive
        Thread.currentThread().join();
    }
}