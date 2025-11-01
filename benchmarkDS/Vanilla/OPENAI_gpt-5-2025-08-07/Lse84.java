import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse84 {
    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        server.createContext("/", new RootHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();

        // 5 test cases
        System.out.println(handlePayload(null)); // Missing -> error
        System.out.println(handlePayload("type: Create")); // Create -> error
        System.out.println(handlePayload("type: Update")); // OK
        System.out.println(handlePayload("name: x")); // OK (no type)
        System.out.println(handlePayload("TYPE: Create")); // OK (case-sensitive key)
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) {
            try {
                URI uri = exchange.getRequestURI();
                Map<String, String> params = parseQuery(uri.getRawQuery());
                String response = processParams(params);
                byte[] bytes = response.getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
                exchange.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(bytes);
                }
            } catch (Exception e) {
                try {
                    String msg = "Internal Server Error";
                    exchange.sendResponseHeaders(500, msg.length());
                    exchange.getResponseBody().write(msg.getBytes(StandardCharsets.UTF_8));
                    exchange.getResponseBody().close();
                } catch (Exception ignored) {}
            }
        }
    }

    public static String processParams(Map<String, String> params) {
        if (params == null || !params.containsKey("payload") || params.get("payload") == null || params.get("payload").isEmpty()) {
            return "Error: missing payload parameter";
        }
        return handlePayload(params.get("payload"));
    }

    public static String handlePayload(String payload) {
        if (payload == null || payload.trim().isEmpty()) {
            return "Error: missing payload parameter";
        }
        String t = yamlGetType(payload);
        if (t != null && "Create".equals(t)) {
            return "Error: payload type Create is not allowed";
        }
        return "OK";
    }

    public static Map<String, String> parseQuery(String rawQuery) {
        Map<String, String> map = new LinkedHashMap<>();
        if (rawQuery == null || rawQuery.isEmpty()) return map;
        for (String pair : rawQuery.split("&")) {
            if (pair.isEmpty()) continue;
            String[] kv = pair.split("=", 2);
            String key = urlDecode(kv[0]);
            String val = kv.length > 1 ? urlDecode(kv[1]) : "";
            map.put(key, val);
        }
        return map;
    }

    public static String urlDecode(String s) {
        if (s == null) return null;
        return URLDecoder.decode(s, StandardCharsets.UTF_8);
    }

    public static String yamlGetType(String payload) {
        if (payload == null) return null;
        String[] lines = payload.replace("\r\n", "\n").replace('\r', '\n').split("\n");
        for (String line : lines) {
            String l = line.trim();
            if (l.isEmpty() || l.startsWith("#")) continue;
            int idx = l.indexOf(':');
            if (idx < 0) continue;
            String key = l.substring(0, idx).trim();
            String val = l.substring(idx + 1).trim();
            if (val.startsWith("\"") && val.endsWith("\"") && val.length() >= 2) {
                val = val.substring(1, val.length() - 1);
            } else if (val.startsWith("'") && val.endsWith("'") && val.length() >= 2) {
                val = val.substring(1, val.length() - 1);
            }
            if ("type".equals(key)) {
                return val;
            }
        }
        return null;
    }
}