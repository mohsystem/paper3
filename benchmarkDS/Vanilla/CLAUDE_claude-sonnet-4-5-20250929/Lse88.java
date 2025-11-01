
import java.io.StringReader;
import java.util.HashMap;
import java.util.Map;
import org.yaml.snakeyaml.Yaml;

public class Lse88 {
    // Product price catalog
    private static final Map<String, Double> PRODUCT_PRICES = new HashMap<>();
    
    static {
        PRODUCT_PRICES.put("PROD001", 10.99);
        PRODUCT_PRICES.put("PROD002", 25.50);
        PRODUCT_PRICES.put("PROD003", 5.75);
        PRODUCT_PRICES.put("PROD004", 100.00);
        PRODUCT_PRICES.put("PROD005", 15.25);
    }
    
    public static String calculatePrice(String yamlPayload) {
        try {
            Yaml yaml = new Yaml();
            Map<String, Object> data = yaml.load(new StringReader(yamlPayload));
            
            String productCode = (String) data.get("product_code");
            int quantity = ((Number) data.get("quantity")).intValue();
            
            if (!PRODUCT_PRICES.containsKey(productCode)) {
                return "Error: Product not found";
            }
            
            double price = PRODUCT_PRICES.get(productCode);
            double totalPrice = price * quantity;
            
            return String.format("%.2f", totalPrice);
        } catch (Exception e) {
            return "Error: Invalid YAML payload - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid product with quantity 2
        String test1 = "product_code: PROD001\\nquantity: 2";
        System.out.println("Test 1: " + calculatePrice(test1));
        
        // Test case 2: Valid product with quantity 5
        String test2 = "product_code: PROD002\\nquantity: 5";
        System.out.println("Test 2: " + calculatePrice(test2));
        
        // Test case 3: Valid product with quantity 1
        String test3 = "product_code: PROD003\\nquantity: 1";
        System.out.println("Test 3: " + calculatePrice(test3));
        
        // Test case 4: Invalid product code
        String test4 = "product_code: PROD999\\nquantity: 3";
        System.out.println("Test 4: " + calculatePrice(test4));
        
        // Test case 5: Valid product with large quantity
        String test5 = "product_code: PROD004\\nquantity: 10";
        System.out.println("Test 5: " + calculatePrice(test5));
    }
}
