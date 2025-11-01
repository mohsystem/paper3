import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class Lse83 {

    // Extract 'name' from JSON payload. Returns null if not present or invalid JSON.
    public static String extractNameFromPayload(String payload) {
        if (payload == null) return null;
        final int n = payload.length();
        int i = 0;

        // Utility lambdas
        class Parser {
            StringBuilder sb = new StringBuilder();

            boolean isWhitespace(char c) {
                return c == ' ' || c == '\n' || c == '\r' || c == '\t';
            }

            void skipWs() {
                while (i < n && isWhitespace(payload.charAt(i))) i++;
            }

            // Parses a JSON string at current index (i) which must point to the opening quote.
            // Returns the decoded string or null if invalid. Advances i to after the closing quote.
            String parseJsonString() {
                if (i >= n || payload.charAt(i) != '"') return null;
                i++; // skip opening "
                sb.setLength(0);
                while (i < n) {
                    char c = payload.charAt(i++);
                    if (c == '"') {
                        return sb.toString();
                    }
                    if (c == '\\') {
                        if (i >= n) return null;
                        char esc = payload.charAt(i++);
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
                                if (i + 4 > n) return null;
                                String hex = payload.substring(i, i + 4);
                                i += 4;
                                try {
                                    int code = Integer.parseInt(hex, 16);
                                    sb.append((char) code);
                                } catch (NumberFormatException e) {
                                    return null;
                                }
                                break;
                            default:
                                return null;
                        }
                    } else {
                        sb.append(c);
                    }
                }
                return null;
            }

            // Parse a non-string JSON literal (number/boolean/null). Returns its raw text value.
            String parseJsonLiteral() {
                int start = i;
                while (i < n) {
                    char c = payload.charAt(i);
                    if (c == ',' || c == '}' || c == ']' || isWhitespace(c)) break;
                    i++;
                }
                if (start == i) return null;
                return payload.substring(start, i).trim();
            }
        }

        Parser p = new Parser();

        while (i < n) {
            char c = payload.charAt(i);
            if (c == '"') {
                int keyStart = i;
                String key = p.parseJsonString();
                if (key == null) return null; // invalid JSON string
                p.skipWs();
                if (i < n && payload.charAt(i) == ':') {
                    i++; // skip colon
                    p.skipWs();
                    if ("name".equals(key)) {
                        if (i < n && payload.charAt(i) == '"') {
                            String val = p.parseJsonString();
                            return val; // may be empty string, that's fine
                        } else {
                            // Non-string value: capture its literal
                            String lit = p.parseJsonLiteral();
                            if (lit != null) return lit;
                            return null;
                        }
                    } else {
                        // Skip the value for non-matching key
                        if (i < n) {
                            if (payload.charAt(i) == '"') {
                                if (p.parseJsonString() == null) return null;
                            } else {
                                p.parseJsonLiteral(); // best-effort skip
                            }
                        }
                    }
                }
            } else {
                i++;
            }
        }
        return null;
    }

    // Escape a string for safe inclusion as a JSON string value
    private static String jsonEscape(String s) {
        if (s == null) return "null";
        StringBuilder sb = new StringBuilder(s.length() + 16);
        sb.append('"');
        for (int k = 0; k < s.length(); k++) {
            char ch = s.charAt(k);
            switch (ch) {
                case '"': sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (ch < 0x20) {
                        sb.append(String.format("\\u%04x", (int) ch));
                    } else {
                        sb.append(ch);
                    }
            }
        }
        sb.append('"');
        return sb.toString();
    }

    // HTTP Handler
    static class NameHandler implements HttpHandler {
        private static final int MAX_BODY = 8192;

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendResponse(exchange, 405, "application/json", "{\"error\":\"Method Not Allowed\"}");
                    return;
                }

                Headers headers = exchange.getRequestHeaders();
                String ct = "";
                if (headers.containsKey("Content-Type")) {
                    ct = headers.getFirst("Content-Type");
                }
                if (ct == null || !ct.toLowerCase(Locale.ROOT).startsWith("application/json")) {
                    sendResponse(exchange, 400, "application/json", "{\"error\":\"Invalid Content-Type\"}");
                    return;
                }

                InputStream is = exchange.getRequestBody();
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[1024];
                int read;
                int total = 0;
                while ((read = is.read(buf)) != -1) {
                    total += read;
                    if (total > MAX_BODY) {
                        sendResponse(exchange, 413, "application/json", "{\"error\":\"Payload Too Large\"}");
                        return;
                    }
                    bos.write(buf, 0, read);
                }
                String payload = bos.toString(StandardCharsets.UTF_8);

                String name = extractNameFromPayload(payload);
                if (name == null) {
                    sendResponse(exchange, 400, "application/json", "{\"error\":\"'name' not found or invalid JSON\"}");
                    return;
                }
                String body = "{\"name\":" + jsonEscape(name) + "}";
                sendResponse(exchange, 200, "application/json; charset=utf-8", body);
            } catch (Exception e) {
                sendResponse(exchange, 500, "application/json", "{\"error\":\"Internal Server Error\"}");
            }
        }

        private void sendResponse(HttpExchange exchange, int status, String contentType, String body) throws IOException {
            byte[] out = body.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", contentType);
            exchange.sendResponseHeaders(status, out.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(out);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // Start HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/name", new NameHandler());
        server.setExecutor(null);
        server.start();

        // 5 test cases for the extraction function
        String[] tests = new String[]{
            "{\"name\":\"Alice\"}",
            "{\"foo\":\"bar\"}",
            "{\"name\":\"Bob \\\"Builder\\\"\"}",
            "{\"name\":\"\"}",
            "{invalid json"
        };
        for (int t = 0; t < tests.length; t++) {
            String res = extractNameFromPayload(tests[t]);
            System.out.println("Test " + (t + 1) + ": " + (res == null ? "null" : res));
        }
        // Keep server running; comment next line if not needed
        // Thread.sleep(Long.MAX_VALUE);
    }
}