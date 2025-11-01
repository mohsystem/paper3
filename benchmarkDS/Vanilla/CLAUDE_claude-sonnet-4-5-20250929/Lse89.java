
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.Map;

public class Lse89 {
    private static Map<String, Double> prices = new HashMap<>();
    
    static {
        prices.put("A001", 10.50);
        prices.put("A002", 15.75);
        prices.put("A003", 20.00);
        prices.put("A004", 25.50);
        prices.put("A005", 30.25);
    }
    
    public static double getPrice(String productCode) {
        return prices.getOrDefault(productCode, 0.0);
    }
    
    public static double calculateTotal(String productCode, int quantity) {
        return getPrice(productCode) * quantity;
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        
        server.createContext("/calculate", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String query = exchange.getRequestURI().getQuery();
                Map<String, String> params = parseQuery(query);
                
                String productCode = params.getOrDefault("code", "");
                int quantity = Integer.parseInt(params.getOrDefault("quantity", "1"));
                
                double total = calculateTotal(productCode, quantity);
                String response = String.format("Product: %s, Quantity: %d, Total: $%.2f", 
                                              productCode, quantity, total);
                
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        });
        
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 5000");
        
        // Test cases
        System.out.println("Test Case 1: " + calculateTotal("A001", 2));
        System.out.println("Test Case 2: " + calculateTotal("A002", 3));
        System.out.println("Test Case 3: " + calculateTotal("A003", 1));
        System.out.println("Test Case 4: " + calculateTotal("A004", 5));
        System.out.println("Test Case 5: " + calculateTotal("A005", 4));
    }
    
    private static Map<String, String> parseQuery(String query) {
        Map<String, String> result = new HashMap<>();
        if (query != null) {
            for (String param : query.split("&")) {
                String[] pair = param.split("=");
                if (pair.length > 1) {
                    result.put(pair[0], pair[1]);
                }
            }
        }
        return result;
    }
}
