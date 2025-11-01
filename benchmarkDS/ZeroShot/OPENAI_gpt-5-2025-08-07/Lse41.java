import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse41 {

    // Build the HTML info page from parameters
    public static String buildInfoPage(String username, String email, String ssn) {
        String safeUser = escapeHtml(safeTruncate(username, 256));
        String safeEmail = escapeHtml(safeTruncate(email, 256));
        String last4 = extractLast4Digits(ssn);
        if (!isValidEmail(safeEmail)) {
            return htmlPage("Invalid input", "<p>Invalid email address.</p>");
        }
        if (safeUser.isEmpty() || last4.isEmpty()) {
            return htmlPage("Invalid input", "<p>Missing or invalid parameters.</p>");
        }
        String body = "<h1>User Info</h1>"
                + "<ul>"
                + "<li>Username: " + safeUser + "</li>"
                + "<li>Email: " + safeEmail + "</li>"
                + "<li>SSN (last 4): " + last4 + "</li>"
                + "</ul>";
        return htmlPage("Info", body);
    }

    private static String htmlPage(String title, String body) {
        return "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>"
                + escapeHtml(title)
                + "</title></head><body>"
                + body
                + "</body></html>";
    }

    private static String safeTruncate(String s, int max) {
        if (s == null) return "";
        if (s.length() <= max) return s;
        return s.substring(0, max);
    }

    private static boolean isValidEmail(String email) {
        if (email == null) return false;
        // Simple robust email check (not RFC complete)
        if (email.length() < 3 || email.length() > 256) return false;
        int at = email.indexOf('@');
        if (at <= 0 || at == email.length() - 1) return false;
        String local = email.substring(0, at);
        String domain = email.substring(at + 1);
        if (domain.indexOf('.') <= 0) return false;
        return local.chars().allMatch(ch -> ch > 31 && ch < 127 && ch != ' ' && ch != '@')
                && domain.chars().allMatch(ch -> ch > 31 && ch < 127 && ch != ' ' && ch != '@');
    }

    private static String extractLast4Digits(String ssn) {
        if (ssn == null) return "";
        StringBuilder digits = new StringBuilder();
        for (int i = ssn.length() - 1; i >= 0; i--) {
            char c = ssn.charAt(i);
            if (Character.isDigit(c)) {
                digits.append(c);
                if (digits.length() == 4) break;
            }
        }
        if (digits.length() < 4) return "";
        return digits.reverse().toString();
    }

    private static String escapeHtml(String s) {
        if (s == null) return "";
        StringBuilder out = new StringBuilder(Math.max(16, s.length()));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#39;"); break;
                default:
                    if (c < 32) out.append(' ');
                    else out.append(c);
            }
        }
        return out.toString();
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> params = new HashMap<>();
        if (query == null || query.isEmpty()) return params;
        String[] pairs = query.split("&");
        for (String pair : pairs) {
            if (pair.isEmpty()) continue;
            int idx = pair.indexOf('=');
            String key = idx > 0 ? pair.substring(0, idx) : pair;
            String val = idx > 0 && pair.length() > idx + 1 ? pair.substring(idx + 1) : "";
            try {
                key = URLDecoder.decode(key, StandardCharsets.UTF_8.name());
                val = URLDecoder.decode(val, StandardCharsets.UTF_8.name());
            } catch (Exception ignored) {}
            params.put(key, val);
        }
        return params;
    }

    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/info", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                        byte[] resp = "Method Not Allowed".getBytes(StandardCharsets.UTF_8);
                        exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=UTF-8");
                        exchange.sendResponseHeaders(405, resp.length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(resp);
                        }
                        return;
                    }
                    String rawQuery = exchange.getRequestURI().getRawQuery();
                    Map<String, String> params = parseQuery(rawQuery);
                    String username = params.getOrDefault("username", "");
                    String email = params.getOrDefault("email", "");
                    String ssn = params.getOrDefault("ssn", "");
                    String body;
                    int status = 200;
                    if (username.isEmpty() || email.isEmpty() || ssn.isEmpty()) {
                        status = 400;
                        body = htmlPage("Bad Request", "<p>Missing required parameters: username, email, ssn.</p>");
                    } else {
                        body = buildInfoPage(username, email, ssn);
                        if (body.contains("Invalid input")) status = 400;
                    }
                    byte[] resp = body.getBytes(StandardCharsets.UTF_8);
                    exchange.getResponseHeaders().add("Content-Type", "text/html; charset=UTF-8");
                    exchange.getResponseHeaders().add("X-Content-Type-Options", "nosniff");
                    exchange.getResponseHeaders().add("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'; img-src 'none'");
                    exchange.sendResponseHeaders(status, resp.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(resp);
                    }
                } finally {
                    exchange.close();
                }
            }
        });
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for the core function
        System.out.println(buildInfoPage("alice", "alice@example.com", "123-45-6789"));
        System.out.println(buildInfoPage("<bob>", "bob@example.org", "0000"));
        System.out.println(buildInfoPage("charlie & co", "charlie@sub.example.com", "xxx-yy-9876"));
        System.out.println(buildInfoPage("dora", "invalid-email", "111-22-3333"));
        System.out.println(buildInfoPage("", "eve@example.net", "4444"));

        // Start server on port 8080
        startServer(8080);
        // Keep running
        Thread.currentThread().join();
    }
}