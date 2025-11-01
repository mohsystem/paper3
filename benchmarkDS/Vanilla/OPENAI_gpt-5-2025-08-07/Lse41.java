import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse41 {

    public static String htmlEscape(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(s.length());
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    public static String last4Digits(String ssn) {
        if (ssn == null) return "";
        StringBuilder digits = new StringBuilder();
        for (char c : ssn.toCharArray()) {
            if (Character.isDigit(c)) digits.append(c);
        }
        String d = digits.toString();
        if (d.length() <= 4) return d;
        return d.substring(d.length() - 4);
    }

    public static String renderInfo(String username, String email, String ssn) {
        String u = htmlEscape(username == null ? "" : username);
        String e = htmlEscape(email == null ? "" : email);
        String l4 = htmlEscape(last4Digits(ssn == null ? "" : ssn));
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>");
        html.append("<html><head><meta charset=\"utf-8\"><title>Info</title></head><body>");
        html.append("<h1>User Info</h1>");
        html.append("<ul>");
        html.append("<li>Username: ").append(u).append("</li>");
        html.append("<li>Email: ").append(e).append("</li>");
        html.append("<li>SSN Last4: ").append(l4).append("</li>");
        html.append("</ul>");
        html.append("</body></html>");
        return html.toString();
    }

    public static Map<String, String> parseQuery(String query) {
        Map<String, String> params = new HashMap<>();
        if (query == null || query.isEmpty()) return params;
        String[] pairs = query.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            try {
                String key = idx >= 0 ? pair.substring(0, idx) : pair;
                String val = idx >= 0 ? pair.substring(idx + 1) : "";
                key = URLDecoder.decode(key, "UTF-8");
                val = URLDecoder.decode(val, "UTF-8");
                params.put(key, val);
            } catch (Exception ignored) {
            }
        }
        return params;
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1);
                return;
            }
            String path = exchange.getRequestURI().getPath();
            if (!"/info".equals(path)) {
                exchange.sendResponseHeaders(404, -1);
                return;
            }
            Map<String, String> params = parseQuery(exchange.getRequestURI().getQuery());
            String username = params.getOrDefault("username", "");
            String email = params.getOrDefault("email", "");
            String ssn = params.getOrDefault("ssn", "");
            String body = renderInfo(username, email, ssn);
            byte[] resp = body.getBytes(StandardCharsets.UTF_8);
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "text/html; charset=utf-8");
            exchange.sendResponseHeaders(200, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        System.out.println(renderInfo("alice", "alice@example.com", "123-45-6789"));
        System.out.println(renderInfo("bob", "bob@example.com", "000112223"));
        System.out.println(renderInfo("charlie", "charlie@company.org", "9999"));
        System.out.println(renderInfo("dora", "dora@mail.net", "111-22-3333"));
        System.out.println(renderInfo("eve", "eve@domain.com", "12-34-56-78-90"));

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        server.start();
    }
}