import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Lse89 {

    private static final Map<String, Double> productPrices = new HashMap<>();

    static {
        productPrices.put("PROD001", 19.99);
        productPrices.put("PROD002", 25.50);
        productPrices.put("PROD003", 5.75);
        productPrices.put("PROD004", 99.99);
    }

    /**
     * Returns the price for a given product code.
     * @param productCode The code of the product.
     * @return An Optional containing the price, or an empty Optional if not found.
     */
    public static Optional<Double> getProductPrice(String productCode) {
        if (productCode == null) {
            return Optional.empty();
        }
        return Optional.ofNullable(productPrices.get(productCode));
    }

    static class PriceHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                return;
            }

            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = parseQuery(query);

            String productCode = params.get("code");
            String quantityStr = params.get("quantity");

            if (productCode == null || productCode.trim().isEmpty() || quantityStr == null) {
                sendResponse(exchange, 400, "{\"error\":\"Missing 'code' or 'quantity' parameter\"}");
                return;
            }

            Optional<Double> priceOpt = getProductPrice(productCode);
            if (!priceOpt.isPresent()) {
                sendResponse(exchange, 404, "{\"error\":\"Product code not found\"}");
                return;
            }

            int quantity;
            try {
                quantity = Integer.parseInt(quantityStr);
                if (quantity <= 0 || quantity > 10000) { // Basic validation
                    sendResponse(exchange, 400, "{\"error\":\"Invalid quantity. Must be between 1 and 10000.\"}");
                    return;
                }
            } catch (NumberFormatException e) {
                sendResponse(exchange, 400, "{\"error\":\"Quantity must be a valid integer\"}");
                return;
            }

            double price = priceOpt.get();
            double totalPrice = price * quantity;

            String response = String.format(
                "{\"product_code\":\"%s\", \"quantity\":%d, \"unit_price\":%.2f, \"total_price\":%.2f}",
                productCode, quantity, price, totalPrice
            );
            sendResponse(exchange, 200, response);
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }

        private Map<String, String> parseQuery(String query) {
            if (query == null || query.isEmpty()) {
                return Collections.emptyMap();
            }
            return Stream.of(query.split("&"))
                         .map(s -> s.split("=", 2))
                         .filter(pair -> pair.length == 2 && !pair[0].isEmpty())
                         .collect(Collectors.toMap(pair -> pair[0], pair -> pair[1], (a, b) -> b));
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 5000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/price", new PriceHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Press Enter to stop the server.");

        System.out.println("\n--- Test Cases ---");
        System.out.println("Run these commands in another terminal:");
        System.out.println("1. Valid product and quantity: curl \"http://localhost:5000/price?code=PROD001&quantity=10\"");
        System.out.println("2. Another valid product: curl \"http://localhost:5000/price?code=PROD004&quantity=2\"");
        System.out.println("3. Invalid product code: curl \"http://localhost:5000/price?code=INVALID&quantity=5\"");
        System.out.println("4. Invalid quantity (non-numeric): curl \"http://localhost:5000/price?code=PROD002&quantity=abc\"");
        System.out.println("5. Missing quantity parameter: curl \"http://localhost:5000/price?code=PROD003\"");
        System.out.println("--------------------\n");
        
        System.in.read();
        server.stop(0);
        System.out.println("Server stopped.");
    }
}