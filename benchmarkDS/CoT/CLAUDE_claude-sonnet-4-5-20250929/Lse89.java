
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.Map;

public class Lse89 {
    private static final Map<String, Double> productPrices = new HashMap<>();
    
    static {
        productPrices.put("P001", 10.99);
        productPrices.put("P002", 25.50);
        productPrices.put("P003", 15.75);
        productPrices.put("P004", 30.00);
        productPrices.put("P005", 5.99);
    }
    
    private static Double getProductPrice(String productCode) {
        if (productCode == null || productCode.trim().isEmpty()) {
            return null;
        }
        // Sanitize input - only allow alphanumeric characters
        if (!productCode.matches("^[A-Za-z0-9]+$")) {
            return null;
        }
        return productPrices.get(productCode.toUpperCase());
    }
    
    private static String calculatePrice(String productCode, String quantityStr) {
        try {
            // Validate inputs
            if (productCode == null || quantityStr == null) {
                return "Error: Missing parameters";
            }
            
            // Validate quantity
            int quantity = Integer.parseInt(quantityStr);
            if (quantity <= 0 || quantity > 10000) {
                return "Error: Invalid quantity. Must be between 1 and 10000";
            }
            
            Double price = getProductPrice(productCode);
            if (price == null) {
                return "Error: Product not found";
            }
            
            double totalPrice = price * quantity;
            return String.format("Product: %s, Quantity: %d, Total Price: $%.2f", 
                               productCode.toUpperCase(), quantity, totalPrice);
        } catch (NumberFormatException e) {
            return "Error: Invalid quantity format";
        } catch (Exception e) {
            return "Error: Processing request failed";
        }
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        
        server.createContext("/calculate", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String query = exchange.getRequestURI().getQuery();
                String response = "Error: Invalid request";
                
                if (query != null) {
                    Map<String, String> params = parseQuery(query);
                    String productCode = params.get("product");
                    String quantity = params.get("quantity");
                    response = calculatePrice(productCode, quantity);
                }
                
                exchange.getResponseHeaders().set("Content-Type", "text/plain");
                exchange.sendResponseHeaders(200, response.getBytes().length);
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        });
        
        server.setExecutor(null);
        server.start();
        System.out.println("Server running on port 5000");
        
        // Test cases
        System.out.println("\\nTest Cases:");
        System.out.println("1. " + calculatePrice("P001", "5"));
        System.out.println("2. " + calculatePrice("P003", "10"));
        System.out.println("3. " + calculatePrice("P999", "5"));
        System.out.println("4. " + calculatePrice("P002", "0"));
        System.out.println("5. " + calculatePrice("P001", "abc"));
    }
    
    private static Map<String, String> parseQuery(String query) {
        Map<String, String> result = new HashMap<>();
        if (query != null) {
            for (String param : query.split("&")) {
                String[] pair = param.split("=");
                if (pair.length == 2) {
                    result.put(pair[0], pair[1]);
                }
            }
        }
        return result;
    }
}
