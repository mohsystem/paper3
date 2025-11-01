import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.net.URI;

import java.nio.charset.StandardCharsets;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Lse84 {
    private static final int PORT = 5000;
    private static final int MAX_BODY_SIZE = 1024 * 1024; // 1 MB

    public static void main(String[] args) throws Exception {
        // 5 test cases
        System.out.println("Test1 (no payload): " + processPayload(null));
        System.out.println("Test2 (Create): " + processPayload("type: Create"));
        System.out.println("Test3 (Update): " + processPayload("type: Update"));
        System.out.println("Test4 (no type): " + processPayload("name: test\nvalue: 123"));
        System.out.println("Test5 (Type with quotes): " + processPayload("type: \"Create\""));

        startServer();
    }

    public static String processPayload(String payload) {
        if (payload == null || payload.trim().isEmpty()) {
            return "Error: missing 'payload' parameter.";
        }
        if (payload.length() > MAX_BODY_SIZE) {
            return "Error: payload too large.";
        }
        String type;
        try {
            type = parseYamlType(payload);
        } catch (Exception e) {
            return "Error: invalid YAML payload.";
        }
        if (type != null && "create".equalsIgnoreCase(type.trim())) {
            return "Error: 'Create' type is not allowed.";
        }
        return "OK";
    }

    private static String parseYamlType(String yaml) {
        String[] lines = yaml.split("\\r?\\n", -1);
        for (String rawLine : lines) {
            String line = stripYamlComments(rawLine);
            line = line.trim();
            if (line.isEmpty() || line.startsWith("---")) {
                continue;
            }
            int idx = line.indexOf(':');
            if (idx <= 0) continue;
            String key = line.substring(0, idx).trim();
            String val = line.substring(idx + 1).trim();
            if (key.equalsIgnoreCase("type")) {
                val = unquote(val);
                return val;
            }
        }
        return null;
    }

    private static String stripYamlComments(String s) {
        boolean inSingle = false, inDouble = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '\'' && !inDouble) inSingle = !inSingle;
            else if (c == '"' && !inSingle) inDouble = !inDouble;
            else if (c == '#' && !inSingle && !inDouble) {
                return s.substring(0, i);
            }
        }
        return s;
    }

    private static String unquote(String s) {
        if (s == null) return null;
        s = s.trim();
        if ((s.startsWith("\"") && s.endsWith("\"")) || (s.startsWith("'") && s.endsWith("'"))) {
            if (s.length() >= 2) {
                return s.substring(1, s.length() - 1);
            }
        }
        return s;
    }

    private static void startServer() throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", PORT), 0);
        server.createContext("/", Lse84::handle);
        server.setExecutor(null);
        System.out.println("Java server listening on port " + PORT);
        server.start();
    }

    private static void handle(HttpExchange exchange) throws IOException {
        try {
            String payloadParam = null;

            // Extract from query string
            URI uri = exchange.getRequestURI();
            String rawQuery = uri.getRawQuery();
            Map<String, List<String>> queryMap = parseQuery(rawQuery);
            if (queryMap.containsKey("payload") && !queryMap.get("payload").isEmpty()) {
                payloadParam = queryMap.get("payload").get(0);
            }

            // If not in query, try body
            if (payloadParam == null && ("POST".equalsIgnoreCase(exchange.getRequestMethod()) || "PUT".equalsIgnoreCase(exchange.getRequestMethod()))) {
                Headers headers = exchange.getRequestHeaders();
                String contentType = "";
                if (headers.getFirst("Content-Type") != null) {
                    contentType = headers.getFirst("Content-Type").split(";")[0].trim().toLowerCase();
                }
                byte[] body = readRequestBody(exchange.getRequestBody(), MAX_BODY_SIZE);
                if (contentType.equals("application/x-www-form-urlencoded")) {
                    Map<String, List<String>> bodyMap = parseQuery(new String(body, StandardCharsets.UTF_8));
                    if (bodyMap.containsKey("payload") && !bodyMap.get("payload").isEmpty()) {
                        payloadParam = bodyMap.get("payload").get(0);
                    }
                } else {
                    // treat entire body as payload
                    if (body.length > 0) {
                        payloadParam = new String(body, StandardCharsets.UTF_8);
                    }
                }
            }

            String result = processPayload(payloadParam);
            boolean isError = result.startsWith("Error:");
            byte[] resp = result.getBytes(StandardCharsets.UTF_8);
            Headers responseHeaders = exchange.getResponseHeaders();
            responseHeaders.set("Content-Type", "text/plain; charset=utf-8");
            responseHeaders.set("X-Content-Type-Options", "nosniff");
            responseHeaders.set("Cache-Control", "no-store");
            exchange.sendResponseHeaders(isError ? 400 : 200, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        } catch (Exception e) {
            byte[] resp = "Error: internal server error.".getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(500, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        } finally {
            exchange.close();
        }
    }

    private static byte[] readRequestBody(InputStream is, int max) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int total = 0;
        int r;
        while ((r = is.read(buf)) != -1) {
            total += r;
            if (total > max) {
                throw new IOException("Body too large");
            }
            baos.write(buf, 0, r);
        }
        return baos.toByteArray();
    }

    private static Map<String, List<String>> parseQuery(String raw) throws IOException {
        Map<String, List<String>> map = new HashMap<>();
        if (raw == null || raw.isEmpty()) return map;
        String[] pairs = raw.split("&");
        for (String pair : pairs) {
            if (pair.isEmpty()) continue;
            String[] kv = pair.split("=", 2);
            String key = urlDecode(kv[0]);
            String val = kv.length > 1 ? urlDecode(kv[1]) : "";
            map.computeIfAbsent(key, k -> new ArrayList<>()).add(val);
        }
        return map;
    }

    private static String urlDecode(String s) throws IOException {
        return URLDecoder.decode(s, StandardCharsets.UTF_8);
    }
}