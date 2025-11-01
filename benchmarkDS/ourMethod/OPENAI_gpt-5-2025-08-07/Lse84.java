import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executors;

public class Lse84 {

    private static final int PORT = 5000;
    private static final int MAX_INPUT_SIZE = 65536; // 64 KiB

    public static void main(String[] args) throws Exception {
        HttpServer server = startServer(PORT);
        Thread.sleep(300);

        System.out.println("Running 5 test cases against http://localhost:" + PORT);

        // 1) Missing payload
        sendAndPrint("GET", "/", null, null);

        // 2) payload with type: Create (should error)
        String y1 = "type: Create\nuser: alice";
        sendAndPrint("GET", "/?payload=" + URLEncoder.encode(y1, StandardCharsets.UTF_8.name()), null, null);

        // 3) payload with type: Update (should succeed)
        String y2 = "type: Update\nid: 42";
        sendAndPrint("GET", "/?payload=" + URLEncoder.encode(y2, StandardCharsets.UTF_8.name()), null, null);

        // 4) invalid YAML (should error)
        String y3 = "type Create\n";
        sendAndPrint("GET", "/?payload=" + URLEncoder.encode(y3, StandardCharsets.UTF_8.name()), null, null);

        // 5) too large payload (should 413)
        char[] big = new char[70000];
        Arrays.fill(big, 'A');
        String y4 = "type: Update\npad: " + new String(big);
        sendAndPrint("POST", "/", "payload=" + URLEncoder.encode(y4, StandardCharsets.UTF_8.name()), "application/x-www-form-urlencoded");

        server.stop(0);
        System.out.println("Server stopped.");
    }

    private static void sendAndPrint(String method, String path, String body, String contentType) {
        HttpURLConnection conn = null;
        try {
            URL url = new URL("http://localhost:" + PORT + path);
            conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod(method);
            conn.setConnectTimeout(3000);
            conn.setReadTimeout(3000);
            if (body != null) {
                conn.setDoOutput(true);
                if (contentType != null) {
                    conn.setRequestProperty("Content-Type", contentType);
                }
                byte[] data = body.getBytes(StandardCharsets.UTF_8);
                conn.setRequestProperty("Content-Length", Integer.toString(data.length));
                conn.getOutputStream().write(data);
            }
            int code = conn.getResponseCode();
            String resp = readAll(conn);
            System.out.println("[" + method + " " + path + "] -> HTTP " + code + " | Body: " + safeSnippet(resp));
        } catch (Exception e) {
            System.out.println("Request failed: " + e.getMessage());
        } finally {
            if (conn != null) conn.disconnect();
        }
    }

    private static String safeSnippet(String s) {
        if (s == null) return "";
        String t = s.replaceAll("\\s+", " ").trim();
        if (t.length() > 120) t = t.substring(0, 120) + "...";
        return t;
    }

    private static String readAll(HttpURLConnection conn) throws IOException {
        try (BufferedReader br = new BufferedReader(new InputStreamReader(
                conn.getResponseCode() >= 400 ? conn.getErrorStream() : conn.getInputStream(),
                StandardCharsets.UTF_8))) {
            StringBuilder sb = new StringBuilder();
            for (String line; (line = br.readLine()) != null; ) {
                sb.append(line).append('\n');
            }
            return sb.toString();
        }
    }

    public static HttpServer startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new RootHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        return server;
    }

    static final class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange ex) throws IOException {
            String method = ex.getRequestMethod();
            Map<String, String> params = new LinkedHashMap<>();
            // Parse query params
            String query = ex.getRequestURI().getRawQuery();
            if (query != null) params.putAll(parseQuery(query));

            // Parse body if present and within limit
            String contentLenStr = ex.getRequestHeaders().getFirst("Content-Length");
            int contentLength = 0;
            if (contentLenStr != null) {
                try {
                    contentLength = Integer.parseInt(contentLenStr.trim());
                } catch (NumberFormatException ignored) {}
            }
            if (contentLength > 0) {
                if (contentLength > MAX_INPUT_SIZE) {
                    send(ex, 413, "Error: payload too large\n");
                    return;
                }
                ByteArrayOutputStream baos = new ByteArrayOutputStream(Math.min(contentLength, MAX_INPUT_SIZE));
                byte[] buf = new byte[4096];
                int remaining = contentLength;
                while (remaining > 0) {
                    int r = ex.getRequestBody().read(buf, 0, Math.min(buf.length, remaining));
                    if (r <= 0) break;
                    baos.write(buf, 0, r);
                    remaining -= r;
                }
                String ctype = safeHeader(ex.getRequestHeaders(), "Content-Type");
                String body = baos.toString(StandardCharsets.UTF_8);
                if (ctype != null && ctype.startsWith("application/x-www-form-urlencoded")) {
                    params.putAll(parseQuery(body));
                } else if (ctype != null && ctype.startsWith("text/plain")) {
                    // If plain text, map entire body to payload if not already provided
                    params.putIfAbsent("payload", body);
                }
            }

            ResponseData resp = processRequest(params);
            send(ex, resp.status, resp.body + "\n");
        }
    }

    private static String safeHeader(Headers h, String name) {
        List<String> vals = h.get(name);
        if (vals == null || vals.isEmpty()) return null;
        return vals.get(0);
    }

    static class ResponseData {
        final int status;
        final String body;
        ResponseData(int s, String b) { this.status = s; this.body = b; }
    }

    public static ResponseData processRequest(Map<String, String> params) {
        String payload = params.get("payload");
        if (payload == null) {
            return new ResponseData(400, "Error: missing payload parameter");
        }
        if (payload.length() > MAX_INPUT_SIZE) {
            return new ResponseData(413, "Error: payload too large");
        }
        Map<String, String> ymap;
        try {
            ymap = parseSimpleYaml(payload);
        } catch (Exception e) {
            return new ResponseData(400, "Error: invalid payload format");
        }
        String type = ymap.get("type");
        if ("Create".equals(type)) {
            return new ResponseData(400, "Error: operation not allowed");
        }
        return new ResponseData(200, "OK");
    }

    private static Map<String, String> parseQuery(String q) {
        if (q == null || q.isEmpty()) return Collections.emptyMap();
        Map<String, String> m = new LinkedHashMap<>();
        String[] pairs = q.split("&");
        for (String p : pairs) {
            int idx = p.indexOf('=');
            String k = idx >= 0 ? p.substring(0, idx) : p;
            String v = idx >= 0 ? p.substring(idx + 1) : "";
            try {
                k = URLDecoder.decode(k, StandardCharsets.UTF_8.name());
                v = URLDecoder.decode(v, StandardCharsets.UTF_8.name());
            } catch (Exception ignored) {}
            if (k.length() <= 100) m.put(k, v);
        }
        return m;
    }

    // Minimal, strict YAML (flat key: value)
    public static Map<String, String> parseSimpleYaml(String src) throws Exception {
        if (src == null) throw new IllegalArgumentException("null");
        if (src.length() > MAX_INPUT_SIZE) throw new IllegalArgumentException("too large");
        Map<String, String> out = new LinkedHashMap<>();
        String[] lines = src.replace("\r\n", "\n").replace('\r', '\n').split("\n", -1);
        int count = 0;
        for (String line : lines) {
            String t = line.trim();
            if (t.isEmpty() || t.startsWith("#")) continue;
            int pos = t.indexOf(':');
            if (pos <= 0) throw new IllegalArgumentException("bad line");
            String key = t.substring(0, pos).trim();
            String val = t.substring(pos + 1).trim();
            if (!key.matches("^[A-Za-z0-9_]{1,64}$")) throw new IllegalArgumentException("bad key");
            if (val.startsWith("\"") && val.endsWith("\"") && val.length() >= 2) {
                val = val.substring(1, val.length() - 1);
            } else if (val.startsWith("'") && val.endsWith("'") && val.length() >= 2) {
                val = val.substring(1, val.length() - 1);
            }
            if (val.length() > 4096) throw new IllegalArgumentException("value too long");
            out.put(key, val);
            count++;
            if (count > 64) throw new IllegalArgumentException("too many keys");
        }
        if (out.isEmpty()) throw new IllegalArgumentException("empty mapping");
        return out;
    }

    private static void send(HttpExchange ex, int status, String body) throws IOException {
        byte[] data = body.getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        ex.sendResponseHeaders(status, data.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(data);
        }
    }
}