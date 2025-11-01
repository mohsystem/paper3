
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import java.util.Map;
import java.util.HashMap;

public class Lse88 {
    private static final Map<String, Double> PRODUCT_PRICES = new HashMap<>();
    
    static {
        PRODUCT_PRICES.put("PROD001", 10.50);
        PRODUCT_PRICES.put("PROD002", 25.00);
        PRODUCT_PRICES.put("PROD003", 15.75);
        PRODUCT_PRICES.put("PROD004", 30.00);
        PRODUCT_PRICES.put("PROD005", 5.99);
    }
    
    public static String calculatePrice(String yamlPayload) {
        try {
            // Use SafeConstructor to prevent code execution vulnerabilities
            Yaml yaml = new Yaml(new SafeConstructor());
            Map<String, Object> data = yaml.load(yamlPayload);
            
            // Validate and extract product_code
            if (!data.containsKey("product_code") || !data.containsKey("quantity")) {
                return "Error: Missing required fields";
            }
            
            String productCode = String.valueOf(data.get("product_code")).trim();
            
            // Validate product code
            if (!PRODUCT_PRICES.containsKey(productCode)) {
                return "Error: Invalid product code";
            }
            
            // Validate and extract quantity
            int quantity;
            try {
                quantity = Integer.parseInt(String.valueOf(data.get("quantity")));
                if (quantity <= 0 || quantity > 10000) {
                    return "Error: Quantity must be between 1 and 10000";
                }
            } catch (NumberFormatException e) {
                return "Error: Invalid quantity format";
            }
            
            // Calculate total price
            double price = PRODUCT_PRICES.get(productCode);
            double totalPrice = price * quantity;
            
            return String.format("%.2f", totalPrice);
            
        } catch (Exception e) {
            return "Error: Invalid YAML format";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid input
        String test1 = "product_code: PROD001\\nquantity: 5";
        System.out.println("Test 1: " + calculatePrice(test1));
        
        // Test case 2: Different product
        String test2 = "product_code: PROD003\\nquantity: 10";
        System.out.println("Test 2: " + calculatePrice(test2));
        
        // Test case 3: Invalid product code
        String test3 = "product_code: INVALID\\nquantity: 3";
        System.out.println("Test 3: " + calculatePrice(test3));
        
        // Test case 4: Invalid quantity
        String test4 = "product_code: PROD002\\nquantity: -5";
        System.out.println("Test 4: " + calculatePrice(test4));
        
        // Test case 5: Missing fields
        String test5 = "product_code: PROD001";
        System.out.println("Test 5: " + calculatePrice(test5));
    }
}
