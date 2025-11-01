
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import com.fasterxml.jackson.dataformat.yaml.YAMLGenerator;
import java.util.HashMap;
import java.util.Map;

public class Lse90 {
    private static final int MAX_QUANTITY = 10000;
    private static final int MAX_YAML_SIZE = 1024;
    
    public static double getPriceForProductCode(String productCode) {
        if (productCode == null || productCode.trim().isEmpty()) {
            return 0.0;
        }
        
        Map<String, Double> prices = new HashMap<>();
        prices.put("PROD001", 10.99);
        prices.put("PROD002", 25.50);
        prices.put("PROD003", 15.75);
        prices.put("PROD004", 8.99);
        prices.put("PROD005", 99.99);
        
        return prices.getOrDefault(productCode.trim().toUpperCase(), 0.0);
    }
    
    public static String calculatePrice(String yamlPayload) {
        try {
            if (yamlPayload == null || yamlPayload.length() > MAX_YAML_SIZE) {
                return createErrorYaml("Invalid payload size");
            }
            
            ObjectMapper mapper = new ObjectMapper(new YAMLFactory());
            Map<String, Object> data = mapper.readValue(yamlPayload, Map.class);
            
            if (!data.containsKey("product_code") || !data.containsKey("quantity")) {
                return createErrorYaml("Missing required fields");
            }
            
            String productCode = String.valueOf(data.get("product_code"));
            int quantity;
            
            try {
                Object qtyObj = data.get("quantity");
                quantity = (qtyObj instanceof Integer) ? (Integer) qtyObj : 
                          Integer.parseInt(String.valueOf(qtyObj));
                
                if (quantity <= 0 || quantity > MAX_QUANTITY) {
                    return createErrorYaml("Invalid quantity");
                }
            } catch (NumberFormatException e) {
                return createErrorYaml("Invalid quantity format");
            }
            
            double unitPrice = getPriceForProductCode(productCode);
            if (unitPrice == 0.0) {
                return createErrorYaml("Product not found");
            }
            
            double totalPrice = unitPrice * quantity;
            
            Map<String, Object> result = new HashMap<>();
            result.put("product_code", productCode);
            result.put("quantity", quantity);
            result.put("unit_price", unitPrice);
            result.put("total_price", totalPrice);
            
            ObjectMapper yamlMapper = new ObjectMapper(
                new YAMLFactory().disable(YAMLGenerator.Feature.WRITE_DOC_START_MARKER)
            );
            return yamlMapper.writeValueAsString(result);
            
        } catch (Exception e) {
            return createErrorYaml("Processing error");
        }
    }
    
    private static String createErrorYaml(String message) {
        try {
            Map<String, String> error = new HashMap<>();
            error.put("error", message);
            ObjectMapper mapper = new ObjectMapper(
                new YAMLFactory().disable(YAMLGenerator.Feature.WRITE_DOC_START_MARKER)
            );
            return mapper.writeValueAsString(error);
        } catch (Exception e) {
            return "error: Processing failed";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid product");
        String yaml1 = "product_code: PROD001\\nquantity: 5";
        System.out.println(calculatePrice(yaml1));
        
        System.out.println("\\nTest Case 2: Different product");
        String yaml2 = "product_code: PROD003\\nquantity: 10";
        System.out.println(calculatePrice(yaml2));
        
        System.out.println("\\nTest Case 3: Invalid product code");
        String yaml3 = "product_code: INVALID\\nquantity: 2";
        System.out.println(calculatePrice(yaml3));
        
        System.out.println("\\nTest Case 4: Invalid quantity");
        String yaml4 = "product_code: PROD002\\nquantity: -5";
        System.out.println(calculatePrice(yaml4));
        
        System.out.println("\\nTest Case 5: Missing fields");
        String yaml5 = "product_code: PROD001";
        System.out.println(calculatePrice(yaml5));
    }
}
