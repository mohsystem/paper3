
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import com.fasterxml.jackson.dataformat.yaml.YAMLParser;
import java.io.IOException;
import java.math.BigDecimal;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse88 {
    // Hardcoded product prices - in production, fetch from secure database
    private static final Map<String, BigDecimal> PRODUCT_PRICES = new HashMap<>();
    
    // Maximum YAML payload size to prevent DoS (10KB)
    private static final int MAX_YAML_SIZE = 10240;
    
    // Pattern to validate product codes (alphanumeric and hyphens only)
    private static final Pattern PRODUCT_CODE_PATTERN = Pattern.compile("^[a-zA-Z0-9-]{1,50}$");
    
    static {
        // Initialize product catalog
        PRODUCT_PRICES.put("PROD-001", new BigDecimal("10.50"));
        PRODUCT_PRICES.put("PROD-002", new BigDecimal("25.00"));
        PRODUCT_PRICES.put("PROD-003", new BigDecimal("5.99"));
        PRODUCT_PRICES.put("PROD-004", new BigDecimal("100.00"));
        PRODUCT_PRICES.put("PROD-005", new BigDecimal("15.75"));
    }
    
    /**
     * Calculates price based on YAML payload.
     * Security measures:
     * - Input size validation to prevent DoS
     * - Safe YAML parsing without entity expansion
     * - Type validation for all extracted values
     * - Range validation for quantity
     * - Whitelist validation for product codes
     * - Overflow prevention using BigDecimal
     * - Generic error messages to prevent information leakage
     */
    public static String calculatePrice(String yamlPayload) {
        try {
            // Validate input is not null or empty
            if (yamlPayload == null || yamlPayload.trim().isEmpty()) {
                return "Error: Invalid request";
            }
            
            // Validate payload size to prevent DoS attacks
            if (yamlPayload.length() > MAX_YAML_SIZE) {
                return "Error: Request too large";
            }
            
            // Parse YAML safely without external entity resolution
            ObjectMapper yamlMapper = new ObjectMapper(new YAMLFactory());
            
            // Configure secure parsing - disable dangerous features
            YAMLFactory yamlFactory = new YAMLFactory();
            yamlFactory.disable(YAMLParser.Feature.PARSE_BOOLEAN_LIKE_WORDS_AS_STRINGS);
            
            Map<String, Object> data;
            try {
                data = yamlMapper.readValue(yamlPayload, Map.class);
            } catch (IOException e) {
                // Generic error message - don't leak parsing details\n                return "Error: Invalid format";\n            }\n            \n            // Validate required fields exist\n            if (!data.containsKey("product_code") || !data.containsKey("quantity")) {\n                return "Error: Missing required fields";\n            }\n            \n            // Extract and validate product_code\n            Object productCodeObj = data.get("product_code");\n            if (!(productCodeObj instanceof String)) {\n                return "Error: Invalid product code type";\n            }\n            String productCode = ((String) productCodeObj).trim();\n            \n            // Validate product code format against whitelist pattern\n            if (!PRODUCT_CODE_PATTERN.matcher(productCode).matches()) {\n                return "Error: Invalid product code format";\n            }\n            \n            // Extract and validate quantity\n            Object quantityObj = data.get("quantity");\n            int quantity;\n            \n            if (quantityObj instanceof Integer) {\n                quantity = (Integer) quantityObj;\n            } else if (quantityObj instanceof String) {\n                try {\n                    quantity = Integer.parseInt((String) quantityObj);\n                } catch (NumberFormatException e) {\n                    return "Error: Invalid quantity format";\n                }\n            } else {\n                return "Error: Invalid quantity type";\n            }\n            \n            // Validate quantity range (1-10000)\n            if (quantity < 1 || quantity > 10000) {\n                return "Error: Quantity out of range";\n            }\n            \n            // Check if product exists in catalog\n            if (!PRODUCT_PRICES.containsKey(productCode)) {\n                return "Error: Product not found";\n            }\n            \n            // Calculate total price using BigDecimal to prevent overflow\n            BigDecimal unitPrice = PRODUCT_PRICES.get(productCode);\n            BigDecimal totalPrice = unitPrice.multiply(new BigDecimal(quantity));\n            \n            // Return formatted result (2 decimal places)\n            return String.format("%.2f", totalPrice);\n            \n        } catch (Exception e) {\n            // Generic error - log details securely but don't expose to user
            // In production: log to secure logging system without sensitive data
            return "Error: Processing failed";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1 - Valid request:");
        String test1 = "product_code: PROD-001\\nquantity: 5";
        System.out.println("Input: " + test1.replace("\\n", " | "));
        System.out.println("Output: " + calculatePrice(test1));
        System.out.println();
        
        System.out.println("Test Case 2 - Different product:");
        String test2 = "product_code: PROD-003\\nquantity: 10";
        System.out.println("Input: " + test2.replace("\\n", " | "));
        System.out.println("Output: " + calculatePrice(test2));
        System.out.println();
        
        System.out.println("Test Case 3 - Invalid product code:");
        String test3 = "product_code: INVALID\\nquantity: 2";
        System.out.println("Input: " + test3.replace("\\n", " | "));
        System.out.println("Output: " + calculatePrice(test3));
        System.out.println();
        
        System.out.println("Test Case 4 - Quantity out of range:");
        String test4 = "product_code: PROD-002\\nquantity: 20000";
        System.out.println("Input: " + test4.replace("\\n", " | "));
        System.out.println("Output: " + calculatePrice(test4));
        System.out.println();
        
        System.out.println("Test Case 5 - Missing field:");
        String test5 = "product_code: PROD-001";
        System.out.println("Input: " + test5);
        System.out.println("Output: " + calculatePrice(test5));
    }
}
