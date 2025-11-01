import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse89 {

    private static final Map<String, Double> productPrices = new HashMap<>();

    static {
        productPrices.put("P100", 10.50);
        productPrices.put("P200", 25.00);
        productPrices.put("P300", 5.75);
        productPrices.put("P400", 19.99);
    }

    /**
     * Returns the price for a given product code.
     * @param productCode The code of the product.
     * @return The price of the product.
     * @throws IllegalArgumentException if the product code is not found.
     */
    public static double getProductPrice(String productCode) {
        if (productPrices.containsKey(productCode)) {
            return productPrices.get(productCode);
        } else {
            throw new IllegalArgumentException("Product code not found: " + productCode);
        }
    }

    public static void main(String[] args) throws IOException {
        // --- 5 Test Cases for the business logic ---
        System.out.println("--- Running Test Cases ---");
        try {
            // Test Case 1: Valid product
            System.out.printf("Test 1: P100 Price: $%.2f%n", getProductPrice("P100"));
            // Test Case 2: Another valid product
            System.out.printf("Test 2: P400 Price: $%.2f%n", getProductPrice("P400"));
            // Test Case 3: Invalid product
            System.out.print("Test 3: P999 Price: ");
            getProductPrice("P999");
        } catch (IllegalArgumentException e) {
            System.out.println(e.getMessage());
        }
        try {
            // Test Case 4: Null product code
             System.out.print("Test 4: null Price: ");
             getProductPrice(null);
        } catch (IllegalArgumentException | NullPointerException e) {
            System.out.println("Product code cannot be null.");
        }
        try {
            // Test Case 5: Empty product code
            System.out.print("Test 5: '' Price: ");
            getProductPrice("");
        } catch (IllegalArgumentException e) {
            System.out.println(e.getMessage());
        }
        System.out.println("--------------------------\n");


        // --- Web App Setup ---
        int port = 5000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/calculate_price", new PriceHandler());
        server.setExecutor(Executors.newCachedThreadPool()); 
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Try accessing: http://localhost:5000/calculate_price?code=P100&quantity=5");
    }

    static class PriceHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "";
            int statusCode = 200;

            try {
                if ("GET".equals(exchange.getRequestMethod())) {
                    Map<String, String> params = queryToMap(exchange.getRequestURI().getQuery());
                    String productCode = params.get("code");
                    String quantityStr = params.get("quantity");

                    if (productCode == null || quantityStr == null) {
                        statusCode = 400;
                        response = "Error: 'code' and 'quantity' parameters are required.";
                    } else {
                        try {
                            int quantity = Integer.parseInt(quantityStr);
                            if (quantity <= 0) {
                                statusCode = 400;
                                response = "Error: Quantity must be a positive integer.";
                            } else {
                                double price = getProductPrice(productCode);
                                double totalPrice = price * quantity;
                                response = String.format("Total price for %d of %s is $%.2f", quantity, productCode, totalPrice);
                            }
                        } catch (NumberFormatException e) {
                            statusCode = 400;
                            response = "Error: 'quantity' must be a valid integer.";
                        } catch (IllegalArgumentException e) {
                            statusCode = 404;
                            response = "Error: " + e.getMessage();
                        }
                    }
                } else {
                    statusCode = 405; // Method Not Allowed
                    response = "Only GET method is allowed.";
                }
            } catch (Exception e) {
                statusCode = 500;
                response = "Internal Server Error.";
            }

            exchange.getResponseHeaders().set("Content-Type", "text/plain");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }

        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null) {
                return result;
            }
            for (String param : query.split("&")) {
                String[] entry = param.split("=");
                if (entry.length > 1) {
                    result.put(entry[0], entry[1]);
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }
    }
}