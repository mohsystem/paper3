import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Lse89 {

    // In-memory database for product prices
    private static final Map<String, Double> productPrices = Map.of(
        "A123", 10.50,
        "B456", 25.00,
        "C789", 5.75,
        "D101", 99.99
    );

    /**
     * Returns the price for a given product code.
     * @param productCode The code of the product.
     * @return An Optional containing the price, or empty if not found.
     */
    public static Optional<Double> getPrice(String productCode) {
        return Optional.ofNullable(productPrices.get(productCode));
    }

    public static void main(String[] args) throws IOException {
        int port = 5000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/price", new PriceHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Java server started on port " + port);
        System.out.println("--- Test Cases (use curl or a browser) ---");
        System.out.println("1. Valid: curl \"http://localhost:5000/price?product_code=A123&quantity=5\"");
        System.out.println("2. Not Found: curl \"http://localhost:5000/price?product_code=X999&quantity=2\"");
        System.out.println("3. Invalid Quantity (text): curl \"http://localhost:5000/price?product_code=B456&quantity=abc\"");
        System.out.println("4. Invalid Quantity (negative): curl \"http://localhost:5000/price?product_code=C789&quantity=-3\"");
        System.out.println("5. Missing Parameter: curl \"http://localhost:5000/price?product_code=A123\"");
    }

    static class PriceHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                return;
            }

            Map<String, String> params = parseQuery(exchange.getRequestURI().getQuery());
            String productCode = params.get("product_code");
            String quantityStr = params.get("quantity");

            if (productCode == null || quantityStr == null) {
                sendResponse(exchange, 400, "{\"error\":\"Missing product_code or quantity parameter\"}");
                return;
            }

            Optional<Double> priceOpt = getPrice(productCode);
            if (priceOpt.isEmpty()) {
                sendResponse(exchange, 404, "{\"error\":\"Product code not found\"}");
                return;
            }

            try {
                int quantity = Integer.parseInt(quantityStr);
                if (quantity <= 0) {
                    sendResponse(exchange, 400, "{\"error\":\"Quantity must be a positive integer\"}");
                    return;
                }

                double totalPrice = priceOpt.get() * quantity;
                String response = String.format("{\"product_code\":\"%s\", \"quantity\":%d, \"total_price\":%.2f}",
                                                productCode, quantity, totalPrice);
                sendResponse(exchange, 200, response);

            } catch (NumberFormatException e) {
                sendResponse(exchange, 400, "{\"error\":\"Quantity must be a valid integer\"}");
            }
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }

        private Map<String, String> parseQuery(String query) {
            Map<String, String> result = new HashMap<>();
            if (query != null) {
                for (String param : query.split("&")) {
                    String[] pair = param.split("=");
                    if (pair.length > 1) {
                        result.put(pair[0], pair[1]);
                    } else {
                        result.put(pair[0], "");
                    }
                }
            }
            return result;
        }
    }
}