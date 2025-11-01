import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.math.RoundingMode;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

public class Lse89 {

    private static final Map<String, BigDecimal> PRICE_MAP;
    static {
        Map<String, BigDecimal> m = new HashMap<>();
        m.put("A100", new BigDecimal("12.50"));
        m.put("B200", new BigDecimal("7.99"));
        m.put("C300", new BigDecimal("199.00"));
        m.put("D400", new BigDecimal("3.25"));
        m.put("E500", new BigDecimal("0.99"));
        PRICE_MAP = Collections.unmodifiableMap(m);
    }

    // Returns the unit price for a product code, or null if not found
    public static BigDecimal getPrice(String code) {
        if (code == null) return null;
        String normalized = code.trim().toUpperCase();
        if (!normalized.matches("^[A-Z0-9]{2,16}$")) {
            return null;
        }
        return PRICE_MAP.get(normalized);
    }

    // Calculates total price; returns null if invalid code or qty
    public static BigDecimal calculateTotal(String code, int qty) {
        if (qty < 1 || qty > 1_000_000) return null;
        BigDecimal unit = getPrice(code);
        if (unit == null) return null;
        return unit.multiply(BigDecimal.valueOf(qty)).setScale(2, RoundingMode.HALF_UP);
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        for (String pair : query.split("&")) {
            int idx = pair.indexOf('=');
            String key = idx > 0 ? pair.substring(0, idx) : pair;
            String val = idx > 0 && pair.length() > idx + 1 ? pair.substring(idx + 1) : "";
            key = URLDecoder.decode(key, StandardCharsets.UTF_8);
            val = URLDecoder.decode(val, StandardCharsets.UTF_8);
            map.put(key, val);
        }
        return map;
    }

    private static void sendJson(HttpExchange exchange, int status, String json) throws IOException {
        Headers headers = exchange.getResponseHeaders();
        headers.set("Content-Type", "application/json; charset=utf-8");
        headers.set("X-Content-Type-Options", "nosniff");
        headers.set("Cache-Control", "no-store");
        byte[] bytes = json.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static String jsonEscape(String s) {
        StringBuilder sb = new StringBuilder();
        for (char c : s.toCharArray()) {
            switch (c) {
                case '"': sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int) c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for the functions
        System.out.println("TEST getPrice A100 -> " + getPrice("A100"));
        System.out.println("TEST calculateTotal A100 x2 -> " + calculateTotal("A100", 2));
        System.out.println("TEST calculateTotal B200 x5 -> " + calculateTotal("B200", 5));
        System.out.println("TEST calculateTotal C300 x1 -> " + calculateTotal("C300", 1));
        System.out.println("TEST invalid code Z999 x3 -> " + calculateTotal("Z999", 3));

        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 5000), 0);
        server.createContext("/price", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendJson(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                    return;
                }
                Map<String, String> q = parseQuery(exchange.getRequestURI().getRawQuery());
                String code = q.getOrDefault("code", "");
                String qtyStr = q.get("qty");
                int qty;
                try {
                    qty = qtyStr == null ? -1 : Integer.parseInt(qtyStr);
                } catch (NumberFormatException e) {
                    sendJson(exchange, 400, "{\"error\":\"Invalid qty\"}");
                    return;
                }

                BigDecimal unit = getPrice(code);
                if (unit == null) {
                    sendJson(exchange, 400, "{\"error\":\"Invalid or unknown code\"}");
                    return;
                }
                if (qty < 1 || qty > 1_000_000) {
                    sendJson(exchange, 400, "{\"error\":\"Invalid qty range\"}");
                    return;
                }
                BigDecimal total = unit.multiply(BigDecimal.valueOf(qty)).setScale(2, RoundingMode.HALF_UP);
                String resp = String.format(
                        "{\"code\":\"%s\",\"qty\":%d,\"unit_price\":\"%s\",\"total_price\":\"%s\"}",
                        jsonEscape(code.trim().toUpperCase()),
                        qty,
                        unit.setScale(2, RoundingMode.HALF_UP).toPlainString(),
                        total.toPlainString()
                );
                sendJson(exchange, 200, resp);
            }
        });
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        System.out.println("Java server running at http://127.0.0.1:5000/price?code=A100&qty=2");
    }
}