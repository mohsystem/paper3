
import java.io.StringReader;
import java.util.HashMap;
import java.util.Map;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.LoaderOptions;
import org.yaml.snakeyaml.constructor.SafeConstructor;

public class Lse88 {
    private static final Map<String, Double> PRODUCT_PRICES = new HashMap<>();
    
    static {
        PRODUCT_PRICES.put("PROD001", 10.50);
        PRODUCT_PRICES.put("PROD002", 25.99);
        PRODUCT_PRICES.put("PROD003", 5.75);
        PRODUCT_PRICES.put("PROD004", 100.00);
        PRODUCT_PRICES.put("PROD005", 15.25);
    }
    
    public static String calculatePrice(String yamlPayload) {
        try {
            if (yamlPayload == null || yamlPayload.trim().isEmpty()) {
                return "Error: Empty payload";
            }
            
            // Use SafeConstructor to prevent code execution vulnerabilities
            LoaderOptions loaderOptions = new LoaderOptions();
            Yaml yaml = new Yaml(new SafeConstructor(loaderOptions));
            
            Map<String, Object> data = yaml.load(yamlPayload);
            
            if (data == null) {
                return "Error: Invalid YAML format";
            }
            
            String productCode = (String) data.get("product_code");
            Object quantityObj = data.get("quantity");
            
            if (productCode == null || quantityObj == null) {
                return "Error: Missing product_code or quantity";
            }
            
            int quantity;
            if (quantityObj instanceof Integer) {
                quantity = (Integer) quantityObj;
            } else if (quantityObj instanceof String) {
                quantity = Integer.parseInt((String) quantityObj);
            } else {
                return "Error: Invalid quantity format";
            }
            
            if (quantity <= 0) {
                return "Error: Quantity must be positive";
            }
            
            if (!PRODUCT_PRICES.containsKey(productCode)) {
                return "Error: Product not found";
            }
            
            double price = PRODUCT_PRICES.get(productCode);
            double totalPrice = price * quantity;
            
            return String.format("%.2f", totalPrice);
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid product and quantity
        String test1 = "product_code: PROD001\\nquantity: 3";
        System.out.println("Test 1: " + calculatePrice(test1));
        
        // Test case 2: Different product
        String test2 = "product_code: PROD002\\nquantity: 2";
        System.out.println("Test 2: " + calculatePrice(test2));
        
        // Test case 3: Invalid product code
        String test3 = "product_code: INVALID\\nquantity: 5";
        System.out.println("Test 3: " + calculatePrice(test3));
        
        // Test case 4: Missing quantity
        String test4 = "product_code: PROD003";
        System.out.println("Test 4: " + calculatePrice(test4));
        
        // Test case 5: Zero quantity
        String test5 = "product_code: PROD004\\nquantity: 0";
        System.out.println("Test 5: " + calculatePrice(test5));
    }
}
