import com.sun.net.httpserver.Headers;
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

    // HTML escape to prevent XSS
    public static String htmlEscape(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder(input.length() + 16);
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                case '/': sb.append("&#x2F;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    // Basic validation functions
    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > 64) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.' )) {
                return false;
            }
        }
        return true;
    }

    public static boolean isValidEmail(String email) {
        if (email == null) return false;
        if (email.length() < 5 || email.length() > 254) return false;
        int at = email.indexOf('@');
        if (at <= 0 || at != email.lastIndexOf('@')) return false;
        String local = email.substring(0, at);
        String domain = email.substring(at + 1);
        if (local.isEmpty() || domain.isEmpty()) return false;
        if (domain.startsWith(".") || domain.endsWith(".")) return false;
        if (!domain.contains(".")) return false;
        // Basic allowed chars check
        String allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._%+-";
        for (int i = 0; i < local.length(); i++) {
            if (allowed.indexOf(local.charAt(i)) < 0) return false;
        }
        for (int i = 0; i < domain.length(); i++) {
            char c = domain.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '.' || c == '-')) return false;
        }
        return true;
    }

    public static String extractLast4Digits(String ssnRaw) {
        if (ssnRaw == null) return null;
        StringBuilder digits = new StringBuilder(9);
        for (int i = 0; i < ssnRaw.length(); i++) {
            char c = ssnRaw.charAt(i);
            if (c >= '0' && c <= '9') {
                digits.append(c);
                if (digits.length() > 16) break; // avoid absurd lengths
            }
        }
        if (digits.length() < 4) return null;
        return digits.substring(digits.length() - 4);
    }

    // Returns HTML page if valid, else throws IllegalArgumentException
    public static String generateInfoPage(String username, String email, String ssn) {
        if (!isValidUsername(username)) {
            throw new IllegalArgumentException("Invalid username");
        }
        if (!isValidEmail(email)) {
            throw new IllegalArgumentException("Invalid email");
        }
        String last4 = extractLast4Digits(ssn);
        if (last4 == null) {
            throw new IllegalArgumentException("Invalid SSN");
        }
        String safeUser = htmlEscape(username);
        String safeEmail = htmlEscape(email);
        String safeLast4 = htmlEscape(last4);

        StringBuilder html = new StringBuilder(256);
        html.append("<!doctype html><html><head><meta charset=\"utf-8\">")
            .append("<title>User Info</title>")
            .append("</head><body>")
            .append("<h1>User Info</h1>")
            .append("<ul>")
            .append("<li>Username: ").append(safeUser).append("</li>")
            .append("<li>Email: ").append(safeEmail).append("</li>")
            .append("<li>SSN (last 4): ").append(safeLast4).append("</li>")
            .append("</ul>")
            .append("</body></html>");
        return html.toString();
    }

    public static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] pairs = query.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            if (idx >= 0) {
                String key = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8);
                String value = URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8);
                map.put(key, value);
            } else {
                String key = URLDecoder.decode(pair, StandardCharsets.UTF_8);
                map.put(key, "");
            }
        }
        return map;
    }

    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 50);
        server.createContext("/info", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                        sendResponse(exchange, 405, "Method Not Allowed");
                        return;
                    }
                    String path = exchange.getRequestURI().getPath();
                    if (!"/info".equals(path)) {
                        sendResponse(exchange, 404, "Not Found");
                        return;
                    }
                    Map<String, String> qp = parseQuery(exchange.getRequestURI().getRawQuery());
                    String username = qp.get("username");
                    String email = qp.get("email");
                    String ssn = qp.get("ssn");
                    if (username == null || email == null || ssn == null) {
                        sendResponse(exchange, 400, "Missing parameters");
                        return;
                    }
                    String body;
                    try {
                        body = generateInfoPage(username, email, ssn);
                    } catch (IllegalArgumentException ex) {
                        sendResponse(exchange, 400, "Invalid input");
                        return;
                    }
                    Headers h = exchange.getResponseHeaders();
                    h.set("Content-Type", "text/html; charset=UTF-8");
                    h.set("X-Content-Type-Options", "nosniff");
                    h.set("X-Frame-Options", "DENY");
                    h.set("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'");
                    byte[] resp = body.getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(200, resp.length);
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
        // Server started; avoid printing sensitive data.
    }

    private static void sendResponse(HttpExchange exchange, int code, String message) throws IOException {
        Headers h = exchange.getResponseHeaders();
        h.set("Content-Type", "text/plain; charset=UTF-8");
        h.set("X-Content-Type-Options", "nosniff");
        h.set("X-Frame-Options", "DENY");
        h.set("Content-Security-Policy", "default-src 'none'");
        byte[] resp = message.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(code, resp.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(resp);
        }
    }

    // Simple tests for function
    public static void runTests() {
        String[][] cases = new String[][] {
            {"alice_01","alice@example.com","123-45-6789"},
            {"bob","bob.smith@example.org","0000"},
            {"charlie.z","charlie.z@example.co.uk","111223333"},
            {"dana-xy","dana-xy@sub.domain.net","***-**-9999"},
            {"safeUser","x.y+z@example.io","42"}
        };
        int passed = 0;
        for (int i = 0; i < cases.length; i++) {
            String u = cases[i][0], e = cases[i][1], s = cases[i][2];
            try {
                String html = generateInfoPage(u, e, s);
                boolean containsScript = html.toLowerCase(Locale.ROOT).contains("<script");
                boolean hasLast4 = extractLast4Digits(s) != null && html.contains(extractLast4Digits(s));
                if (!containsScript && hasLast4) {
                    passed++;
                }
            } catch (Exception ex) {
                // Case 5 expected to fail because only 2 digits; treat as pass if exception for last case
                if (i == 4) passed++;
            }
        }
        // no sensitive output
        // System.out.println("Tests passed: " + passed + "/5");
    }

    public static void main(String[] args) throws Exception {
        runTests();
        startServer(8080);
    }
}