
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse89 {
    // Whitelist of valid product codes with their prices
    private static final Map<String, Double> PRODUCT_PRICES = new HashMap<>();
    
    static {
        // Initialize with predefined product codes and prices
        PRODUCT_PRICES.put("PROD001", 10.50);
        PRODUCT_PRICES.put("PROD002", 25.00);
        PRODUCT_PRICES.put("PROD003", 15.75);
        PRODUCT_PRICES.put("PROD004", 30.00);
        PRODUCT_PRICES.put("PROD005", 8.99);
    }
    
    // Input validation pattern for product codes (alphanumeric, max 20 chars)
    private static final Pattern PRODUCT_CODE_PATTERN = Pattern.compile("^[A-Za-z0-9]{1,20}$");
    
    /**
     * Returns the price for a given product code.
     * @param productCode The product code to look up
     * @return The price, or null if not found
     */
    public static Double getPrice(String productCode) {
        // Input validation: check for null or empty
        if (productCode == null || productCode.isEmpty()) {
            return null;
        }
        
        // Input validation: check format against whitelist pattern
        if (!PRODUCT_CODE_PATTERN.matcher(productCode).matches()) {
            return null;
        }
        
        // Return price from secure map (returns null if not found)
        return PRODUCT_PRICES.get(productCode);
    }
    
    /**
     * Calculates total price for a product code and quantity.
     * @param productCode The product code
     * @param quantity The quantity (must be positive, max 10000)
     * @return The total price, or null if invalid
     */
    public static Double calculatePrice(String productCode, int quantity) {
        // Input validation: quantity must be positive and within reasonable limits
        if (quantity <= 0 || quantity > 10000) {
            return null;
        }
        
        Double price = getPrice(productCode);
        if (price == null) {
            return null;
        }
        
        // Check for integer overflow before multiplication
        if (price > Double.MAX_VALUE / quantity) {
            return null;
        }
        
        return price * quantity;
    }
    
    static class PriceHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "";
            int statusCode = 400;
            
            try {
                // Only allow GET requests
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    response = "Method not allowed";
                    statusCode = 405;
                } else {
                    String query = exchange.getRequestURI().getQuery();
                    
                    // Input validation: query string must exist
                    if (query == null || query.isEmpty()) {
                        response = "Missing parameters";
                        statusCode = 400;
                    } else {
                        String productCode = null;
                        Integer quantity = null;
                        
                        // Parse query parameters safely
                        String[] params = query.split("&");
                        for (String param : params) {
                            // Limit parameter count to prevent DOS
                            if (params.length > 10) {
                                response = "Too many parameters";
                                statusCode = 400;
                                break;
                            }
                            
                            String[] keyValue = param.split("=", 2);
                            if (keyValue.length == 2) {
                                String key = keyValue[0];
                                String value = keyValue[1];
                                
                                // Input validation: limit parameter name and value length
                                if (key.length() > 50 || value.length() > 100) {
                                    response = "Parameter too long";
                                    statusCode = 400;
                                    break;
                                }
                                
                                if ("code".equals(key)) {
                                    productCode = value;
                                } else if ("quantity".equals(key)) {
                                    try {
                                        quantity = Integer.parseInt(value);
                                    } catch (NumberFormatException e) {
                                        // Invalid quantity format
                                        response = "Invalid quantity format";
                                        statusCode = 400;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        // Process request if parameters are valid
                        if (productCode != null && quantity != null && statusCode == 400) {
                            Double totalPrice = calculatePrice(productCode, quantity);
                            if (totalPrice != null) {
                                // Use constant format string for output
                                response = String.format("Total price: %.2f", totalPrice);
                                statusCode = 200;
                            } else {
                                response = "Invalid product code or quantity";
                                statusCode = 400;
                            }
                        } else if (statusCode == 400 && response.isEmpty()) {
                            response = "Missing required parameters";
                        }
                    }
                }
            } catch (Exception e) {
                // Generic error message, no internal details leaked
                response = "Server error";
                statusCode = 500;
            }
            
            // Set security headers
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test cases
            System.out.println("Test Case 1: " + calculatePrice("PROD001", 2)); // 21.0
            System.out.println("Test Case 2: " + calculatePrice("PROD002", 3)); // 75.0
            System.out.println("Test Case 3: " + calculatePrice("PROD003", 5)); // 78.75
            System.out.println("Test Case 4: " + calculatePrice("INVALID", 1)); // null
            System.out.println("Test Case 5: " + calculatePrice("PROD001", -1)); // null
            
            // Create HTTP server on port 5000
            HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
            server.createContext("/price", new PriceHandler());
            server.setExecutor(null);
            server.start();
            System.out.println("Server started on port 5000");
            System.out.println("Example: http://localhost:5000/price?code=PROD001&quantity=2");
        } catch (IOException e) {
            System.err.println("Failed to start server");
        }
    }
}
