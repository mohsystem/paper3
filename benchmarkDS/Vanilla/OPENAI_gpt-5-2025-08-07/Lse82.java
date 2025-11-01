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
import java.util.*;

public class Lse82 {

    // Parse URL query string into a map
    public static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] pairs = query.split("&");
        for (String p : pairs) {
            if (p.isEmpty()) continue;
            String[] kv = p.split("=", 2);
            String key = urlDecode(kv[0]);
            String val = kv.length > 1 ? urlDecode(kv[1]) : "";
            map.put(key, val);
        }
        return map;
    }

    // URL decode helper
    public static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, "UTF-8");
        } catch (Exception e) {
            return s;
        }
    }

    // Minimal JSON unescape for strings
    private static String unescapeJsonString(String s) {
        StringBuilder out = new StringBuilder();
        boolean esc = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (esc) {
                switch (c) {
                    case '"': out.append('"'); break;
                    case '\\': out.append('\\'); break;
                    case '/': out.append('/'); break;
                    case 'b': out.append('\b'); break;
                    case 'f': out.append('\f'); break;
                    case 'n': out.append('\n'); break;
                    case 'r': out.append('\r'); break;
                    case 't': out.append('\t'); break;
                    case 'u':
                        if (i + 4 < s.length()) {
                            String hex = s.substring(i + 1, i + 5);
                            try {
                                int cp = Integer.parseInt(hex, 16);
                                out.append((char) cp);
                                i += 4;
                            } catch (NumberFormatException ex) {
                                out.append("\\u").append(hex);
                                i += 4;
                            }
                        } else {
                            out.append("\\u");
                        }
                        break;
                    default:
                        out.append(c);
                }
                esc = false;
            } else if (c == '\\') {
                esc = true;
            } else {
                out.append(c);
            }
        }
        return out.toString();
    }

    // Parse a flat JSON object into a Map<String,String>. Supports string/number/bool/null values.
    public static Map<String, String> parseJsonToMap(String json) {
        Map<String, String> map = new HashMap<>();
        if (json == null) return map;
        String s = json.trim();
        if (s.startsWith("{")) s = s.substring(1);
        if (s.endsWith("}")) s = s.substring(0, s.length() - 1);

        List<String> pairs = new ArrayList<>();
        StringBuilder cur = new StringBuilder();
        boolean inQuotes = false;
        boolean esc = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (inQuotes) {
                cur.append(c);
                if (esc) {
                    esc = false;
                } else if (c == '\\') {
                    esc = true;
                } else if (c == '"') {
                    inQuotes = false;
                }
            } else {
                if (c == '"') {
                    inQuotes = true;
                    cur.append(c);
                } else if (c == ',') {
                    pairs.add(cur.toString());
                    cur.setLength(0);
                } else {
                    cur.append(c);
                }
            }
        }
        if (cur.length() > 0) pairs.add(cur.toString());

        for (String pair : pairs) {
            String p = pair.trim();
            if (p.isEmpty()) continue;
            int colon = -1;
            inQuotes = false;
            esc = false;
            for (int i = 0; i < p.length(); i++) {
                char c = p.charAt(i);
                if (inQuotes) {
                    if (esc) {
                        esc = false;
                    } else if (c == '\\') {
                        esc = true;
                    } else if (c == '"') {
                        inQuotes = false;
                    }
                } else {
                    if (c == '"') {
                        inQuotes = true;
                    } else if (c == ':') {
                        colon = i;
                        break;
                    }
                }
            }
            if (colon < 0) continue;
            String k = p.substring(0, colon).trim();
            String v = p.substring(colon + 1).trim();

            if (k.startsWith("\"") && k.endsWith("\"") && k.length() >= 2) {
                k = k.substring(1, k.length() - 1);
            }
            String val;
            if (v.startsWith("\"") && v.endsWith("\"") && v.length() >= 2) {
                val = unescapeJsonString(v.substring(1, v.length() - 1));
            } else {
                val = v;
            }
            map.put(k, val);
        }
        return map;
    }

    // Extract "name" from payload JSON
    public static String getNameFromPayload(String payload) {
        Map<String, String> m = parseJsonToMap(payload);
        String name = m.get("name");
        return name == null ? "" : name;
    }

    // HTTP handler for /api
    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed");
                    return;
                }
                URI uri = exchange.getRequestURI();
                Map<String, String> params = parseQuery(uri.getRawQuery());
                String payload = params.get("payload");
                if (payload == null) {
                    send(exchange, 400, "Missing payload parameter");
                    return;
                }
                String name = getNameFromPayload(payload);
                send(exchange, 200, name);
            } catch (Exception e) {
                send(exchange, 500, "Server Error");
            }
        }

        private void send(HttpExchange ex, int status, String body) throws IOException {
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            Headers h = ex.getResponseHeaders();
            h.set("Content-Type", "text/plain; charset=utf-8");
            ex.sendResponseHeaders(status, bytes.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for payload parsing
        String[] tests = new String[] {
                "{\"name\":\"Alice\"}",
                "{\"name\":\"Bob\",\"age\":30}",
                "{\"age\":22,\"name\":\"Carol\"}",
                "{\"name\":\"D\\\"E\"}",
                "{\"name\":\"Eve Adams\",\"city\":\"NY\"}"
        };
        for (String t : tests) {
            System.out.println(getNameFromPayload(t));
        }

        // Start web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server running on http://localhost:8080/api?payload=%7B%22name%22%3A%22World%22%7D");
    }
}