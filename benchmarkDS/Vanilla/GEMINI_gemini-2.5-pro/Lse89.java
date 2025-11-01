import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

public class Lse89 {

    private static final Map<String, Double> productPrices = new HashMap<>();

    static {
        // Initialize product prices
        productPrices.put("P101", 10.50);
        productPrices.put("P205", 25.00);
        productPrices.put("P330", 5.75);
        productPrices.put("P412", 99.99);
        productPrices.put("P555", 1.25);
    }

    /**
     * Returns the price for a given product code.
     * @param productCode The code of the product.
     * @return The price of the product, or 0.0 if not found.
     */
    public static double getPrice(String productCode) {
        return productPrices.getOrDefault(productCode, 0.0);
    }

    static class PriceHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            String[] segments = path.split("/");
            String response;
            int statusCode = 200;

            // Expecting path like /<product_code>/<quantity>
            if (segments.length == 3) {
                String productCode = segments[1];
                try {
                    int quantity = Integer.parseInt(segments[2]);
                    double unitPrice = getPrice(productCode);

                    if (unitPrice > 0) {
                         if (quantity > 0) {
                            double totalPrice = unitPrice * quantity;
                            response = String.format(
                                "{\"product_code\": \"%s\", \"quantity\": %d, \"unit_price\": %.2f, \"total_price\": %.2f}",
                                productCode, quantity, unitPrice, totalPrice
                            );
                        } else {
                            statusCode = 400;
                            response = "{\"error\": \"Quantity must be a positive integer.\"}";
                        }
                    } else {
                        statusCode = 404;
                        response = "{\"error\": \"Product code not found.\"}";
                    }
                } catch (NumberFormatException e) {
                    statusCode = 400;
                    response = "{\"error\": \"Invalid quantity format. Must be an integer.\"}";
                }
            } else {
                statusCode = 400;
                response = "{\"error\": \"Invalid URL format. Use /<product_code>/<quantity>\"}";
            }

            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        server.createContext("/", new PriceHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        System.out.println("Java server started on port 5000.");
        System.out.println("--- Test Cases ---");
        System.out.println("1. Valid: curl http://localhost:5000/P101/10");
        System.out.println("2. Valid: curl http://localhost:5000/P412/3");
        System.out.println("3. Invalid Product: curl http://localhost:5000/P999/5");
        System.out.println("4. Invalid Quantity (text): curl http://localhost:5000/P205/abc");
        System.out.println("5. Invalid Quantity (zero): curl http://localhost:5000/P330/0");
    }
}