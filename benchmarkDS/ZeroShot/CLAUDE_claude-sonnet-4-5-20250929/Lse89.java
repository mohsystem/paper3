
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.HashMap;
import java.util.Map;

public class Lse89 {
    private static Map<String, Double> productPrices = new HashMap<>();
    
    static {
        productPrices.put("PROD001", 10.99);
        productPrices.put("PROD002", 25.50);
        productPrices.put("PROD003", 5.75);
        productPrices.put("PROD004", 100.00);
        productPrices.put("PROD005", 15.25);
    }
    
    public static double getPrice(String productCode) {
        return productPrices.getOrDefault(productCode, 0.0);
    }
    
    public static double calculatePrice(String productCode, int quantity) {
        if (quantity < 0) {
            return 0.0;
        }
        double price = getPrice(productCode);
        return price * quantity;
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        
        server.createContext("/calculate", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String response = "";
                int statusCode = 200;
                
                try {
                    URI uri = exchange.getRequestURI();
                    String query = uri.getQuery();
                    
                    if (query != null) {
                        Map<String, String> params = parseQuery(query);
                        String productCode = params.get("productCode");
                        String quantityStr = params.get("quantity");
                        
                        if (productCode != null && quantityStr != null) {
                            int quantity = Integer.parseInt(quantityStr);
                            double totalPrice = calculatePrice(productCode, quantity);
                            response = String.format("{\\"productCode\\": \\"%s\\", \\"quantity\\": %d, \\"totalPrice\\": %.2f}", 
                                                    productCode, quantity, totalPrice);
                        } else {
                            response = "{\\"error\\": \\"Missing productCode or quantity parameter\\"}";
                            statusCode = 400;
                        }
                    } else {
                        response = "{\\"error\\": \\"No query parameters provided\\"}";
                        statusCode = 400;
                    }
                } catch (Exception e) {
                    response = "{\\"error\\": \\"" + e.getMessage() + "\\"}";
                    statusCode = 500;
                }
                
                exchange.getResponseHeaders().add("Content-Type", "application/json");
                exchange.sendResponseHeaders(statusCode, response.length());
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
        System.out.println("1. Product PROD001, Quantity 5: " + calculatePrice("PROD001", 5));
        System.out.println("2. Product PROD002, Quantity 3: " + calculatePrice("PROD002", 3));
        System.out.println("3. Product PROD003, Quantity 10: " + calculatePrice("PROD003", 10));
        System.out.println("4. Product PROD999, Quantity 2: " + calculatePrice("PROD999", 2));
        System.out.println("5. Product PROD004, Quantity 0: " + calculatePrice("PROD004", 0));
    }
    
    private static Map<String, String> parseQuery(String query) {
        Map<String, String> result = new HashMap<>();
        for (String param : query.split("&")) {
            String[] pair = param.split("=");
            if (pair.length > 1) {
                result.put(pair[0], pair[1]);
            }
        }
        return result;
    }
}
