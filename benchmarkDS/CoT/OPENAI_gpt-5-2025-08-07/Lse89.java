import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.math.RoundingMode;
import java.net.InetSocketAddress;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse89 {
    // Chain-of-Through process:
    // 1) Problem understanding: build simple web app with endpoint to compute price*quantity, and a function to return unit price by product code.
    // 2) Security requirements: validate inputs (code format/length, qty range), safe parsing, minimal headers, avoid reflection/dependencies.
    // 3) Secure coding generation: careful query parsing, strict patterns, proper content-type, error handling, no data leaks.
    // 4) Code review: inline comments and checks ensure safety, bounds, and correctness.
    // 5) Secure code output: final mitigations included.

    private static final Pattern CODE_PATTERN = Pattern.compile("^[A-Za-z0-9_-]{1,20}$");
    private static final Map<String, BigDecimal> PRICES;
    static {
        Map<String, BigDecimal> m = new HashMap<>();
        m.put("A100", new BigDecimal("9.99"));
        m.put("B200", new BigDecimal("14.50"));
        m.put("C300", new BigDecimal("4.25"));
        m.put("D400", new BigDecimal("100.00"));
        m.put("E500", new BigDecimal("0.99"));
        PRICES = Collections.unmodifiableMap(m);
    }

    // Function: returns the price for a product code (or null if not found)
    public static BigDecimal getPrice(String code) {
        if (code == null || !CODE_PATTERN.matcher(code).matches()) {
            return null;
        }
        return PRICES.get(code);
    }

    // Calculate total price given code and quantity
    public static BigDecimal calculateTotal(String code, int qty) {
        BigDecimal unit = getPrice(code);
        if (unit == null) return null;
        if (qty < 1 || qty > 1_000_000) return null;
        return unit.multiply(new BigDecimal(qty)).setScale(2, RoundingMode.HALF_UP);
    }

    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", port), 0);
        server.createContext("/price", new PriceHandler());
        server.setExecutor(Executors.newFixedThreadPool(8));
        server.start();
        System.out.println("Java server started on port " + port);
    }

    static class PriceHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String method = exchange.getRequestMethod();
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "application/json; charset=utf-8");
            headers.set("X-Content-Type-Options", "nosniff");
            headers.set("Cache-Control", "no-store");

            if (!"GET".equalsIgnoreCase(method)) {
                String resp = "{\"error\":\"method_not_allowed\"}";
                exchange.sendResponseHeaders(405, resp.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }

            String path = exchange.getRequestURI().getPath();
            if (!"/price".equals(path)) {
                String resp = "{\"error\":\"not_found\"}";
                exchange.sendResponseHeaders(404, resp.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }

            Map<String, String> params = parseQuery(exchange.getRequestURI().getRawQuery());
            String code = params.get("code");
            String qtyStr = params.get("qty");
            if (code == null || qtyStr == null) {
                String resp = "{\"error\":\"missing_parameters\"}";
                exchange.sendResponseHeaders(400, resp.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }

            if (!CODE_PATTERN.matcher(code).matches()) {
                String resp = "{\"error\":\"invalid_code\"}");
                String fixed = "{\"error\":\"invalid_code\"}";
                exchange.sendResponseHeaders(400, fixed.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(fixed.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }

            int qty;
            try {
                qty = Integer.parseInt(qtyStr);
                if (qty < 1 || qty > 1_000_000) throw new NumberFormatException("out_of_range");
            } catch (NumberFormatException ex) {
                String resp = "{\"error\":\"invalid_quantity\"}";
                exchange.sendResponseHeaders(400, resp.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }

            BigDecimal unit = getPrice(code);
            if (unit == null) {
                String resp = "{\"error\":\"unknown_code\"}";
                exchange.sendResponseHeaders(404, resp.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }
            BigDecimal total = unit.multiply(new BigDecimal(qty)).setScale(2, RoundingMode.HALF_UP);
            String json = "{\"code\":\"" + code + "\",\"quantity\":" + qty + ",\"unit_price\":" + unit.toPlainString() + ",\"total\":" + total.toPlainString() + "}";
            exchange.sendResponseHeaders(200, json.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(json.getBytes(StandardCharsets.UTF_8));
            }
        }

        private Map<String, String> parseQuery(String raw) {
            Map<String, String> map = new HashMap<>();
            if (raw == null || raw.length() == 0) return map;
            String[] pairs = raw.split("&");
            for (String p : pairs) {
                int idx = p.indexOf('=');
                if (idx > 0) {
                    String key = URLDecoder.decode(p.substring(0, idx), StandardCharsets.UTF_8);
                    String val = URLDecoder.decode(p.substring(idx + 1), StandardCharsets.UTF_8);
                    map.put(key, val);
                }
            }
            return map;
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases (function tests)
        System.out.println("Test1 A100 x2 => " + calculateTotal("A100", 2));
        System.out.println("Test2 B200 x1 => " + calculateTotal("B200", 1));
        System.out.println("Test3 C300 x5 => " + calculateTotal("C300", 5));
        System.out.println("Test4 D400 x10 => " + calculateTotal("D400", 10));
        System.out.println("Test5 Z999 x3 => " + calculateTotal("Z999", 3));

        // Start server on port 5000
        startServer(5000);

        // Optionally perform simple HTTP test requests (non-blocking demo)
        try {
            String[] urls = {
                "http://127.0.0.1:5000/price?code=A100&qty=3",
                "http://127.0.0.1:5000/price?code=B200&qty=1",
                "http://127.0.0.1:5000/price?code=C300&qty=5",
                "http://127.0.0.1:5000/price?code=D400&qty=2",
                "http://127.0.0.1:5000/price?code=UNKNOWN&qty=2"
            };
            for (String u : urls) {
                HttpURLConnection conn = (HttpURLConnection) new URL(u).openConnection();
                conn.setRequestMethod("GET");
                conn.setConnectTimeout(1000);
                conn.setReadTimeout(1000);
                try (java.io.InputStream is = conn.getInputStream()) {
                    byte[] b = is.readAllBytes();
                    System.out.println(new String(b, StandardCharsets.UTF_8));
                } catch (IOException ioe) {
                    try (java.io.InputStream es = conn.getErrorStream()) {
                        if (es != null) {
                            byte[] b = es.readAllBytes();
                            System.out.println(new String(b, StandardCharsets.UTF_8));
                        }
                    }
                }
                conn.disconnect();
            }
        } catch (Exception ignore) {}

        // Keep running
        Thread.currentThread().join();
    }
}