import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse82 {
    // Maximum allowed sizes for inputs
    private static final int MAX_QUERY_LEN = 4096;
    private static final int MAX_JSON_LEN = 2048;
    private static final int MAX_NAME_LEN = 256;

    public static void main(String[] args) throws Exception {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null);
        Thread t = new Thread(server::start);
        t.start();

        // Give the server a moment to start
        Thread.sleep(300);

        // 5 test cases
        String base = "http://127.0.0.1:" + port + "/api";

        // 1) Valid payload with name
        System.out.println(httpGet(base + "?payload=" + URLEncoder.encode("{\"name\":\"Alice\"}", "UTF-8")));

        // 2) Valid JSON with extra fields
        System.out.println(httpGet(base + "?payload=" + URLEncoder.encode("{\"name\":\"Bob\",\"age\":\"30\"}", "UTF-8")));

        // 3) Missing payload parameter
        System.out.println(httpGet(base));

        // 4) Invalid JSON
        System.out.println(httpGet(base + "?payload=" + URLEncoder.encode("{\"name\":Bob}", "UTF-8")));

        // 5) Missing name field
        System.out.println(httpGet(base + "?payload=" + URLEncoder.encode("{\"title\":\"Engineer\"}", "UTF-8")));

        server.stop(0);
    }

    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendResponse(exchange, 405, "Method Not Allowed");
                    return;
                }
                String path = exchange.getRequestURI().getPath();
                if (!"/api".equals(path)) {
                    sendResponse(exchange, 404, "Not Found");
                    return;
                }

                String query = exchange.getRequestURI().getRawQuery();
                if (query == null || query.length() == 0 || query.length() > MAX_QUERY_LEN) {
                    sendResponse(exchange, 400, "Bad Request");
                    return;
                }

                String payloadParam = getQueryParam(query, "payload");
                if (payloadParam == null) {
                    sendResponse(exchange, 400, "Bad Request");
                    return;
                }

                String json = urlDecode(payloadParam);
                if (json == null || json.length() == 0 || json.length() > MAX_JSON_LEN) {
                    sendResponse(exchange, 400, "Bad Request");
                    return;
                }

                Map<String, String> map;
                try {
                    map = parseSimpleJson(json);
                } catch (Exception e) {
                    sendResponse(exchange, 400, "Bad Request");
                    return;
                }

                String name = map.get("name");
                if (name == null || name.length() == 0 || name.length() > MAX_NAME_LEN) {
                    sendResponse(exchange, 400, "Bad Request");
                    return;
                }

                sendResponse(exchange, 200, name);
            } catch (Exception ex) {
                sendResponse(exchange, 500, "Internal Server Error");
            }
        }
    }

    private static void sendResponse(HttpExchange exchange, int status, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    // Extract a query parameter value by name from the raw query (no decoding of entire query to avoid memory issues)
    private static String getQueryParam(String rawQuery, String key) {
        String[] pairs = rawQuery.split("&");
        for (String p : pairs) {
            int idx = p.indexOf('=');
            if (idx <= 0) continue;
            String k = p.substring(0, idx);
            if (k.equals(key)) {
                return p.substring(idx + 1);
            }
        }
        return null;
    }

    private static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return null;
        }
    }

    // Minimal JSON parser for flat objects with string keys and string values only.
    // Supports escaped quotes (\") and backslashes (\\). Whitespace is allowed.
    private static Map<String, String> parseSimpleJson(String json) throws Exception {
        Map<String, String> map = new HashMap<>();
        int i = 0, n = json.length();

        while (i < n && Character.isWhitespace(json.charAt(i))) i++;
        if (i >= n || json.charAt(i) != '{') throw new Exception("Invalid JSON");
        i++;
        while (true) {
            while (i < n && Character.isWhitespace(json.charAt(i))) i++;
            if (i < n && json.charAt(i) == '}') { i++; break; }

            // key
            if (i >= n || json.charAt(i) != '"') throw new Exception("Invalid JSON key");
            String key = parseJsonString(json, i);
            i = nextIndexAfterString(json, i);

            while (i < n && Character.isWhitespace(json.charAt(i))) i++;
            if (i >= n || json.charAt(i) != ':') throw new Exception("Invalid JSON colon");
            i++;
            while (i < n && Character.isWhitespace(json.charAt(i))) i++;

            // value (string only)
            if (i >= n || json.charAt(i) != '"') throw new Exception("Invalid JSON value");
            String val = parseJsonString(json, i);
            i = nextIndexAfterString(json, i);

            map.put(key, val);

            while (i < n && Character.isWhitespace(json.charAt(i))) i++;
            if (i < n && json.charAt(i) == ',') {
                i++;
                continue;
            } else if (i < n && json.charAt(i) == '}') {
                i++;
                break;
            } else if (i >= n) {
                throw new Exception("Unterminated object");
            } else {
                throw new Exception("Invalid separator");
            }
        }

        while (i < n && Character.isWhitespace(json.charAt(i))) i++;
        if (i != n) throw new Exception("Trailing data");
        return map;
    }

    private static String parseJsonString(String s, int startQuoteIdx) throws Exception {
        if (s.charAt(startQuoteIdx) != '"') throw new Exception("Expected quote");
        StringBuilder out = new StringBuilder();
        int i = startQuoteIdx + 1;
        while (i < s.length()) {
            char c = s.charAt(i);
            if (c == '"') {
                return out.toString();
            } else if (c == '\\') {
                if (i + 1 >= s.length()) throw new Exception("Bad escape");
                char e = s.charAt(i + 1);
                switch (e) {
                    case '"':
                    case '\\':
                    case '/': out.append(e); break;
                    case 'b': out.append('\b'); break;
                    case 'f': out.append('\f'); break;
                    case 'n': out.append('\n'); break;
                    case 'r': out.append('\r'); break;
                    case 't': out.append('\t'); break;
                    case 'u':
                        if (i + 6 > s.length()) throw new Exception("Bad unicode escape");
                        String hex = s.substring(i + 2, i + 6);
                        int code;
                        try { code = Integer.parseInt(hex, 16); }
                        catch (NumberFormatException ex) { throw new Exception("Bad unicode"); }
                        out.append((char) code);
                        i += 4; // additional advance due to \uXXXX
                        break;
                    default:
                        throw new Exception("Bad escape");
                }
                i += 2;
            } else {
                out.append(c);
                i++;
            }
        }
        throw new Exception("Unterminated string");
    }

    private static int nextIndexAfterString(String s, int startQuoteIdx) throws Exception {
        if (s.charAt(startQuoteIdx) != '"') throw new Exception("Expected quote");
        int i = startQuoteIdx + 1;
        while (i < s.length()) {
            char c = s.charAt(i);
            if (c == '"') return i + 1;
            if (c == '\\') {
                i++;
                if (i >= s.length()) throw new Exception("Bad escape");
                char e = s.charAt(i);
                if (e == 'u') i += 4;
            }
            i++;
        }
        throw new Exception("Unterminated string");
    }

    private static String httpGet(String urlStr) throws IOException {
        HttpURLConnection conn = null;
        try {
            URL url = new URL(urlStr);
            conn = (HttpURLConnection) url.openConnection();
            conn.setInstanceFollowRedirects(false);
            conn.setRequestMethod("GET");
            conn.setConnectTimeout(2000);
            conn.setReadTimeout(2000);
            int code = conn.getResponseCode();
            byte[] buf;
            if (code >= 200 && code < 300) {
                buf = conn.getInputStream().readAllBytes();
            } else {
                buf = conn.getErrorStream() != null ? conn.getErrorStream().readAllBytes() : new byte[0];
            }
            String body = new String(buf, StandardCharsets.UTF_8);
            return code + ":" + body;
        } finally {
            if (conn != null) conn.disconnect();
        }
    }
}