
import org.yaml.snakebeans.Yaml;
import java.util.HashMap;
import java.util.Map;

public class Lse90 {
    private static final Map<String, Double> productPrices = new HashMap<>();
    
    static {
        productPrices.put("PROD001", 10.99);
        productPrices.put("PROD002", 25.50);
        productPrices.put("PROD003", 15.75);
        productPrices.put("PROD004", 8.99);
        productPrices.put("PROD005", 100.00);
    }
    
    public static Double getPriceForProductCode(String productCode) {
        if (productCode == null || productCode.trim().isEmpty()) {
            return null;
        }
        return productPrices.getOrDefault(productCode.trim().toUpperCase(), null);
    }
    
    public static String calculatePrice(String yamlPayload) {
        if (yamlPayload == null || yamlPayload.trim().isEmpty()) {
            return createErrorResponse("Invalid payload");
        }
        
        try {
            Yaml yaml = new Yaml();
            Map<String, Object> data = yaml.load(yamlPayload);
            
            if (data == null || !data.containsKey("product_code") || !data.containsKey("quantity")) {
                return createErrorResponse("Missing required fields");
            }
            
            String productCode = String.valueOf(data.get("product_code"));
            int quantity;
            
            try {
                quantity = Integer.parseInt(String.valueOf(data.get("quantity")));
                if (quantity <= 0) {
                    return createErrorResponse("Quantity must be positive");
                }
            } catch (NumberFormatException e) {
                return createErrorResponse("Invalid quantity format");
            }
            
            Double price = getPriceForProductCode(productCode);
            if (price == null) {
                return createErrorResponse("Product not found");
            }
            
            double totalPrice = price * quantity;
            
            Map<String, Object> response = new HashMap<>();
            response.put("product_code", productCode);
            response.put("quantity", quantity);
            response.put("unit_price", price);
            response.put("total_price", totalPrice);
            
            return yaml.dump(response);
            
        } catch (Exception e) {
            return createErrorResponse("Error processing request");
        }
    }
    
    private static String createErrorResponse(String message) {
        Map<String, String> error = new HashMap<>();
        error.put("error", message);
        Yaml yaml = new Yaml();
        return yaml.dump(error);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid request");
        String result1 = calculatePrice("product_code: PROD001\\nquantity: 5");
        System.out.println(result1);
        System.out.println();
        
        System.out.println("Test Case 2: Different product");
        String result2 = calculatePrice("product_code: PROD003\\nquantity: 3");
        System.out.println(result2);
        System.out.println();
        
        System.out.println("Test Case 3: Invalid product code");
        String result3 = calculatePrice("product_code: INVALID\\nquantity: 2");
        System.out.println(result3);
        System.out.println();
        
        System.out.println("Test Case 4: Missing fields");
        String result4 = calculatePrice("product_code: PROD001");
        System.out.println(result4);
        System.out.println();
        
        System.out.println("Test Case 5: Negative quantity");
        String result5 = calculatePrice("product_code: PROD002\\nquantity: -5");
        System.out.println(result5);
    }
}
