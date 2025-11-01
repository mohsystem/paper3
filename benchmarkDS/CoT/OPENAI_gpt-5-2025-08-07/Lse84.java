import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.*;

/*
Chain-of-Through in code (comments only; no external output):
1) Problem understanding:
   - Build a simple web server on port 5000.
   - On each request, check for "payload" parameter; if missing, return error.
   - If present, parse YAML safely (restricted), read key "type".
   - If type == "Create", return error message; otherwise success.
2) Security requirements:
   - Limit input sizes to prevent DOS.
   - Avoid unsafe YAML features by using a minimal parser for simple key:value pairs only.
   - Properly decode and validate inputs; handle unsupported methods.
3) Secure coding generation:
   - Implement minimal, strict YAML parsing and query parsing.
   - Validate lengths, handle exceptions, return clear error messages with proper HTTP codes.
4) Code review (embedded as defensive checks):
   - Check for null/empty values, size caps, decoding errors.
   - Use UTF-8 consistently and avoid reflection/dynamic code execution.
5) Secure code output:
   - Final implementation below follows the above safeguards.
*/

public class Lse84 {

    // Core logic: process payload YAML and decide response text.
    public static String processPayload(String payload) {
        if (payload == null) {
            return "Error: Missing payload parameter";
        }
        // Enforce size limit (10 KB)
        if (payload.length() > 10_000) {
            return "Error: Payload too large";
        }
        Map<String, String> map = parseSimpleYamlMap(payload);
        if (map == null) {
            return "Error: Invalid YAML";
        }
        String type = map.get("type");
        if ("Create".equals(type)) {
            return "Error: Create type is not allowed";
        }
        return "OK";
    }

    // Minimal safe YAML parser: only supports flat "key: value" pairs, no nesting, no anchors/tags.
    private static Map<String, String> parseSimpleYamlMap(String yaml) {
        if (yaml == null) return null;
        Map<String, String> result = new LinkedHashMap<>();
        String[] lines = yaml.split("\n", -1);
        if (lines.length > 1000) return null; // cap lines
        int total = 0;
        for (String rawLine : lines) {
            total += rawLine.length();
            if (total > 10_000) return null; // cap total content processed
            String line = rawLine.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            int idx = line.indexOf(':');
            if (idx <= 0) return null; // invalid: no key or starts with colon
            String key = line.substring(0, idx).trim();
            String value = line.substring(idx + 1).trim();
            if (key.isEmpty()) return null;
            value = unquote(value);
            // For safety, disallow YAML features like "&", "*", "!" at start of value
            if (!value.isEmpty()) {
                char c = value.charAt(0);
                if (c == '&' || c == '*' || c == '!') return null;
            }
            result.put(key, value);
        }
        return result;
    }

    private static String unquote(String v) {
        if (v == null) return null;
        if ((v.startsWith("\"") && v.endsWith("\"")) || (v.startsWith("'") && v.endsWith("'"))) {
            return v.substring(1, v.length() - 1);
        }
        return v;
        }

    // HTTP server handler
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) {
            String response;
            int status;
            try {
                String method = exchange.getRequestMethod();
                if (!"GET".equalsIgnoreCase(method) && !"POST".equalsIgnoreCase(method)) {
                    writeResponse(exchange, 405, "Error: Method Not Allowed");
                    return;
                }

                String payloadParam = null;

                // Try query string first
                URI uri = exchange.getRequestURI();
                String rawQuery = uri.getRawQuery();
                if (rawQuery != null) {
                    Map<String, String> params = parseQuery(rawQuery);
                    payloadParam = params.get("payload");
                }

                // For POST with form data, attempt to read if not found in query
                if (payloadParam == null && "POST".equalsIgnoreCase(method)) {
                    byte[] bodyBytes = readBody(exchange, 10_240); // 10 KB cap
                    String contentType = Optional.ofNullable(exchange.getRequestHeaders().getFirst("Content-Type")).orElse("");
                    String body = new String(bodyBytes, StandardCharsets.UTF_8);
                    if (contentType.startsWith("application/x-www-form-urlencoded")) {
                        Map<String, String> params = parseQuery(body);
                        payloadParam = params.get("payload");
                    } else {
                        // If raw body provided, treat entire body as payload for convenience
                        if (!body.isEmpty()) {
                            payloadParam = body;
                        }
                    }
                }

                if (payloadParam == null) {
                    response = "Error: Missing payload parameter";
                    status = 400;
                } else {
                    String result = processPayload(payloadParam);
                    response = result;
                    status = result.startsWith("Error:") ? 400 : 200;
                }
                writeResponse(exchange, status, response);
            } catch (Exception e) {
                writeResponse(exchange, 500, "Error: Internal Server Error");
            }
        }
    }

    private static Map<String, String> parseQuery(String raw) {
        Map<String, String> map = new LinkedHashMap<>();
        if (raw == null || raw.isEmpty()) return map;
        String[] pairs = raw.split("&");
        for (String p : pairs) {
            if (p.isEmpty()) continue;
            String name;
            String val;
            int idx = p.indexOf('=');
            if (idx >= 0) {
                name = p.substring(0, idx);
                val = p.substring(idx + 1);
            } else {
                name = p;
                val = "";
            }
            try {
                name = URLDecoder.decode(name, StandardCharsets.UTF_8.name());
                val = URLDecoder.decode(val, StandardCharsets.UTF_8.name());
            } catch (Exception ignored) {}
            if (!map.containsKey(name)) {
                // enforce value length cap
                if (val.length() > 10_000) {
                    val = val.substring(0, 10_000);
                }
                map.put(name, val);
            }
        }
        return map;
    }

    private static byte[] readBody(HttpExchange exchange, int maxBytes) {
        try (BufferedReader br = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))) {
            char[] buf = new char[2048];
            int n;
            int remaining = maxBytes;
            StringBuilder sb = new StringBuilder();
            while ((n = br.read(buf, 0, Math.min(buf.length, remaining))) != -1) {
                sb.append(buf, 0, n);
                remaining -= n;
                if (remaining <= 0) break;
            }
            return sb.toString().getBytes(StandardCharsets.UTF_8);
        } catch (Exception e) {
            return new byte[0];
        }
    }

    private static void writeResponse(HttpExchange exchange, int status, String body) {
        try {
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(status, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        } catch (Exception ignored) {}
    }

    public static void main(String[] args) throws Exception {
        // Start HTTP server on port 5000
        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", 5000), 0);
        server.createContext("/", new RootHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server listening on http://0.0.0.0:5000/");

        // 5 test cases for core logic
        String[] tests = new String[] {
                "type: Create",
                "type: Update",
                "foo: bar",
                "type: 'Create'",
                "type Create"
        };
        for (int i = 0; i < tests.length; i++) {
            String in = tests[i];
            String out = processPayload(in);
            System.out.println("Test " + (i + 1) + " => " + out);
        }
    }
}