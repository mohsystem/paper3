import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Lse132 {

    private static final String HELLO_HTML = "<!doctype html>\n"
            + "<html lang=\"en\">\n"
            + "<head>\n"
            + "  <meta charset=\"utf-8\">\n"
            + "  <title>Hello</title>\n"
            + "</head>\n"
            + "<body>\n"
            + "  <h1>Hello, {{username}}</h1>\n"
            + "</body>\n"
            + "</html>\n";

    public static String htmlEscape(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(Math.min(1024, s.length() * 2));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    public static String renderHello(String username) {
        String safe = htmlEscape(username == null ? "" : username);
        return HELLO_HTML.replace("{{username}}", safe);
    }

    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed");
                    return;
                }
                String rawPath = exchange.getRequestURI().getPath();
                if (rawPath == null || !rawPath.startsWith("/hello/")) {
                    send(exchange, 404, "Not Found");
                    return;
                }
                String usernamePart = rawPath.substring("/hello/".length());
                String username = URLDecoder.decode(usernamePart, StandardCharsets.UTF_8);
                if (username.length() > 100) {
                    send(exchange, 400, "Username too long");
                    return;
                }
                String body = renderHello(username);
                Headers headers = exchange.getResponseHeaders();
                headers.set("Content-Type", "text/html; charset=utf-8");
                headers.set("X-Content-Type-Options", "nosniff");
                headers.set("X-Frame-Options", "DENY");
                headers.set("Content-Security-Policy", "default-src 'none'; style-src 'self' 'unsafe-inline'");
                byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(bytes);
                }
            } finally {
                exchange.close();
            }
        }

        private void send(HttpExchange exchange, int code, String message) throws IOException {
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "text/plain; charset=utf-8");
            headers.set("X-Content-Type-Options", "nosniff");
            headers.set("X-Frame-Options", "DENY");
            headers.set("Content-Security-Policy", "default-src 'none'");
            byte[] bytes = message.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(code, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for renderHello
        String[] tests = new String[] {
                "Alice",
                "Bob",
                "Eve<script>",
                "李雷",
                "O'Reilly & Co"
        };
        for (String t : tests) {
            System.out.println(renderHello(t));
            System.out.println("----");
        }

        // Start minimal HTTP server for /hello/<username>
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/hello", exchange -> {
            // Enforce trailing slash pattern /hello/<username>
            String path = exchange.getRequestURI().getPath();
            if ("/hello".equals(path) || "/hello/".equals(path)) {
                Headers headers = exchange.getResponseHeaders();
                headers.set("Content-Type", "text/plain; charset=utf-8");
                byte[] msg = "Usage: /hello/<username>".getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(400, msg.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(msg);
                } finally {
                    exchange.close();
                }
            } else {
                new HelloHandler().handle(exchange);
            }
        });
        server.setExecutor(null);
        server.start();
        System.out.println("Java server listening on http://127.0.0.1:8080/hello/<username>");
    }
}