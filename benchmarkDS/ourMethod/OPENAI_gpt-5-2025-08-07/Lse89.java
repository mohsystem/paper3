import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class Lse89 {

    // Immutable product price list
    private static final Map<String, BigDecimal> PRICES;
    static {
        Map<String, BigDecimal> m = new HashMap<>();
        m.put("P100", new BigDecimal("10.00"));
        m.put("P200", new BigDecimal("25.50"));
        m.put("P300", new BigDecimal("99.99"));
        m.put("P400", new BigDecimal("5.25"));
        m.put("P500", new BigDecimal("1.50"));
        PRICES = Collections.unmodifiableMap(m);
    }

    // Validate product code: alphanumeric only, 1..20 chars
    private static boolean isValidCode(String code) {
        if (code == null) return false;
        if (code.length() < 1 || code.length() > 20) return false;
        for (int i = 0; i < code.length(); i++) {
            char c = code.charAt(i);
            if (!((c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9'))) {
                return false;
            }
        }
        return true;
    }

    // Parse and validate quantity: integer 1..100000
    private static Integer parseQuantity(String qtyStr) {
        if (qtyStr == null || qtyStr.length() == 0 || qtyStr.length() > 9) return null;
        for (int i = 0; i < qtyStr.length(); i++) {
            char c = qtyStr.charAt(i);
            if (c < '0' || c > '9') return null;
        }
        try {
            int q = Integer.parseInt(qtyStr);
            if (q < 1 || q > 100000) return null;
            return q;
        } catch (NumberFormatException e) {
            return null;
        }
    }

    // Function: return price for a product code, or null if invalid/unknown
    public static BigDecimal getPriceForCode(String code) {
        if (!isValidCode(code)) return null;
        return PRICES.get(code);
    }

    // Function: calculate total for product+qty, or null on invalid input
    public static BigDecimal calculateTotal(String code, int qty) {
        if (!isValidCode(code)) return null;
        if (qty < 1 || qty > 100000) return null;
        BigDecimal price = PRICES.get(code);
        if (price == null) return null;
        return price.multiply(BigDecimal.valueOf(qty));
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> params = new HashMap<>();
        if (query == null || query.isEmpty()) return params;
        String[] parts = query.split("&");
        for (String part : parts) {
            if (part.isEmpty()) continue;
            String[] kv = part.split("=", 2);
            String key = URLDecoder.decode(kv[0], StandardCharsets.UTF_8);
            String val = kv.length > 1 ? URLDecoder.decode(kv[1], StandardCharsets.UTF_8) : "";
            params.put(key, val);
        }
        return params;
    }

    private static class PriceHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendJson(exchange, 405, "{\"error\":\"method not allowed\"}");
                    return;
                }
                URI uri = exchange.getRequestURI();
                if (uri == null) {
                    sendJson(exchange, 400, "{\"error\":\"invalid input\"}");
                    return;
                }
                Map<String, String> params = parseQuery(uri.getRawQuery());
                String code = params.get("code");
                String qtyStr = params.get("qty");

                if (!isValidCode(code)) {
                    sendJson(exchange, 400, "{\"error\":\"invalid input\"}");
                    return;
                }
                Integer qty = parseQuantity(qtyStr);
                if (qty == null) {
                    sendJson(exchange, 400, "{\"error\":\"invalid input\"}");
                    return;
                }
                BigDecimal price = PRICES.get(code);
                if (price == null) {
                    sendJson(exchange, 404, "{\"error\":\"not found\"}");
                    return;
                }
                BigDecimal total = price.multiply(BigDecimal.valueOf(qty));
                String json = new StringBuilder(128)
                        .append("{\"code\":\"").append(code).append("\",")
                        .append("\"unitPrice\":").append(price.toPlainString()).append(",")
                        .append("\"quantity\":").append(qty).append(",")
                        .append("\"total\":").append(total.toPlainString()).append("}")
                        .toString();
                sendJson(exchange, 200, json);
            } catch (Exception ex) {
                sendJson(exchange, 500, "{\"error\":\"server error\"}");
            }
        }

        private void sendJson(HttpExchange exchange, int status, String body) throws IOException {
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "application/json; charset=utf-8");
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(status, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 simple test cases for functions (unit tests)
        System.out.println("Test1 total P100 x 3 = " + calculateTotal("P100", 3)); // 30.00
        System.out.println("Test2 total P200 x 2 = " + calculateTotal("P200", 2)); // 51.00
        System.out.println("Test3 price P300 = " + getPriceForCode("P300")); // 99.99
        System.out.println("Test4 invalid code returns null = " + getPriceForCode("..bad.."));
        System.out.println("Test5 invalid qty returns null = " + calculateTotal("P100", 0));

        // Start web app on port 5000
        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", 5000), 0);
        server.createContext("/price", new PriceHandler());
        server.setExecutor(null); // default single-threaded
        System.out.println("Server running on http://0.0.0.0:5000");
        server.start();
    }
}