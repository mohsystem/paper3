import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse83 {

    // Extract 'name' from JSON or form-urlencoded payload
    public static String extractName(String payload) {
        if (payload == null) return null;
        payload = payload.trim();
        // Try JSON pattern: {"name":"value"}
        Pattern jsonPattern = Pattern.compile("\"name\"\\s*:\\s*\"((?:\\\\.|[^\"\\\\])*)\"");
        Matcher m = jsonPattern.matcher(payload);
        if (m.find()) {
            String val = m.group(1);
            // Unescape common JSON escapes
            val = val.replace("\\\"", "\"")
                     .replace("\\\\", "\\")
                     .replace("\\n", "\n")
                     .replace("\\r", "\r")
                     .replace("\\t", "\t");
            return val;
        }
        // Try x-www-form-urlencoded: name=value
        int idx = payload.indexOf("name=");
        if (idx >= 0) {
            int start = idx + 5;
            int end = payload.indexOf('&', start);
            String val = (end >= 0) ? payload.substring(start, end) : payload.substring(start);
            try {
                return URLDecoder.decode(val, StandardCharsets.UTF_8);
            } catch (Exception e) {
                return val;
            }
        }
        return null;
    }

    // Simple HTTP handler
    static class ExtractHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String method = exchange.getRequestMethod();
            String payload = "";
            if ("POST".equalsIgnoreCase(method) || "PUT".equalsIgnoreCase(method) || "PATCH".equalsIgnoreCase(method)) {
                try (InputStream is = exchange.getRequestBody()) {
                    payload = new String(is.readAllBytes(), StandardCharsets.UTF_8);
                }
            } else if ("GET".equalsIgnoreCase(method)) {
                String query = exchange.getRequestURI().getQuery();
                payload = query == null ? "" : query;
            }
            String name = extractName(payload);
            byte[] resp;
            int code;
            if (name != null) {
                resp = name.getBytes(StandardCharsets.UTF_8);
                code = 200;
            } else {
                String msg = "Field 'name' not found";
                resp = msg.getBytes(StandardCharsets.UTF_8);
                code = 400;
            }
            exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(code, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // Start HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/extract", new ExtractHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server running at http://localhost:8080/extract");

        // 5 test cases for the extractName function
        String[] tests = new String[] {
                "{\"name\":\"Alice\"}",
                "{\"age\":30}",
                "{\"name\":\"Bob\",\"x\":1}",
                "name=Carol&x=1",
                "{\"greet\":\"hi\",\"name\":\"A\\\"B C\"}"
        };
        for (int i = 0; i < tests.length; i++) {
            String result = extractName(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + (result != null ? result : "null"));
        }
    }
}