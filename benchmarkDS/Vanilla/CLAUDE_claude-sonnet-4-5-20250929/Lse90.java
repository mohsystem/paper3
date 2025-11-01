
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import java.util.HashMap;
import java.util.Map;

public class Lse90 {
    
    public static double getPriceForProductCode(String productCode) {
        Map<String, Double> prices = new HashMap<>();
        prices.put("PROD001", 10.99);
        prices.put("PROD002", 25.50);
        prices.put("PROD003", 5.75);
        prices.put("PROD004", 100.00);
        prices.put("PROD005", 15.25);
        
        return prices.getOrDefault(productCode, 0.0);
    }
    
    public static String calculatePrice(String yamlPayload) {
        try {
            ObjectMapper mapper = new ObjectMapper(new YAMLFactory());
            
            @SuppressWarnings("unchecked")
            Map<String, Object> data = mapper.readValue(yamlPayload, Map.class);
            
            String productCode = (String) data.get("product_code");
            int quantity = (int) data.get("quantity");
            
            double price = getPriceForProductCode(productCode);
            double totalPrice = price * quantity;
            
            Map<String, Object> result = new HashMap<>();
            result.put("product_code", productCode);
            result.put("quantity", quantity);
            result.put("total_price", totalPrice);
            
            return mapper.writeValueAsString(result);
        } catch (Exception e) {
            return "error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String yaml1 = "product_code: PROD001\\nquantity: 2";
        System.out.println("Test 1:\\n" + calculatePrice(yaml1));
        System.out.println();
        
        // Test case 2
        String yaml2 = "product_code: PROD002\\nquantity: 5";
        System.out.println("Test 2:\\n" + calculatePrice(yaml2));
        System.out.println();
        
        // Test case 3
        String yaml3 = "product_code: PROD003\\nquantity: 10";
        System.out.println("Test 3:\\n" + calculatePrice(yaml3));
        System.out.println();
        
        // Test case 4
        String yaml4 = "product_code: PROD004\\nquantity: 1";
        System.out.println("Test 4:\\n" + calculatePrice(yaml4));
        System.out.println();
        
        // Test case 5
        String yaml5 = "product_code: PROD999\\nquantity: 3";
        System.out.println("Test 5:\\n" + calculatePrice(yaml5));
    }
}
