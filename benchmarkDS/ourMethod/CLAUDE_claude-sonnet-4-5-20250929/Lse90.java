
// Lse90.java
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.LoaderOptions;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.DumperOptions;

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse90 {
    // Maximum payload size to prevent memory exhaustion (1KB)
    private static final int MAX_PAYLOAD_SIZE = 1024;
    
    // Pattern to validate product codes (alphanumeric, 1-20 chars)
    private static final Pattern PRODUCT_CODE_PATTERN = Pattern.compile("^[a-zA-Z0-9]{1,20}$");
    
    // Maximum quantity to prevent integer overflow
    private static final int MAX_QUANTITY = 1000000;
    
    // Hardcoded price database (in production, use a secure database)
    private static final Map<String, Double> PRICE_DATABASE = new HashMap<>();
    
    static {
        PRICE_DATABASE.put("PROD001", 10.50);
        PRICE_DATABASE.put("PROD002", 25.99);
        PRICE_DATABASE.put("PROD003", 5.75);
        PRICE_DATABASE.put("PROD004", 100.00);
        PRICE_DATABASE.put("PROD005", 15.25);
    }
    
    /**
     * Returns the price for a given product code.
     * Uses input validation to prevent injection attacks.
     */
    public static double getPriceForProductCode(String productCode) {
        // Input validation: check null and empty
        if (productCode == null || productCode.trim().isEmpty()) {
            throw new IllegalArgumentException("Product code cannot be null or empty");
        }
        
        // Input validation: check format (alphanumeric only)
        if (!PRODUCT_CODE_PATTERN.matcher(productCode).matches()) {
            throw new IllegalArgumentException("Invalid product code format");
        }
        
        // Check if product exists
        if (!PRICE_DATABASE.containsKey(productCode)) {
            throw new IllegalArgumentException("Product code not found");
        }
        
        return PRICE_DATABASE.get(productCode);
    }
    
    /**
     * Calculates total price from YAML payload.
     * Uses SafeConstructor to prevent arbitrary code execution during YAML deserialization.
     */
    public static String calculatePrice(String yamlPayload) {
        try {
            // Input validation: check null and size limits
            if (yamlPayload == null) {
                return createErrorResponse("Payload cannot be null");
            }
            
            if (yamlPayload.length() > MAX_PAYLOAD_SIZE) {
                return createErrorResponse("Payload exceeds maximum size");
            }
            
            // Use SafeConstructor to prevent arbitrary object instantiation (CWE-502)
            LoaderOptions loaderOptions = new LoaderOptions();
            Yaml yaml = new Yaml(new SafeConstructor(loaderOptions));
            
            // Parse YAML safely
            Object parsedData = yaml.load(yamlPayload);
            
            // Type validation: ensure it's a Map
            if (!(parsedData instanceof Map)) {
                return createErrorResponse("Invalid payload format");
            }
            
            @SuppressWarnings("unchecked")
            Map<String, Object> data = (Map<String, Object>) parsedData;
            
            // Extract and validate product_code
            Object productCodeObj = data.get("product_code");
            if (productCodeObj == null || !(productCodeObj instanceof String)) {
                return createErrorResponse("Missing or invalid product_code");
            }
            String productCode = ((String) productCodeObj).trim();
            
            // Extract and validate quantity
            Object quantityObj = data.get("quantity");
            if (quantityObj == null) {
                return createErrorResponse("Missing quantity");
            }
            
            int quantity;
            try {
                if (quantityObj instanceof Integer) {
                    quantity = (Integer) quantityObj;
                } else if (quantityObj instanceof String) {
                    quantity = Integer.parseInt((String) quantityObj);
                } else {
                    return createErrorResponse("Invalid quantity type");
                }
            } catch (NumberFormatException e) {
                return createErrorResponse("Invalid quantity format");
            }
            
            // Validate quantity range
            if (quantity <= 0 || quantity > MAX_QUANTITY) {
                return createErrorResponse("Quantity out of valid range");
            }
            
            // Get price for product code (with validation inside)
            double unitPrice = getPriceForProductCode(productCode);
            
            // Calculate total with overflow check
            double totalPrice = unitPrice * quantity;
            
            // Check for overflow (though double has wide range)
            if (Double.isInfinite(totalPrice) || Double.isNaN(totalPrice)) {
                return createErrorResponse("Calculation overflow");
            }
            
            // Create response with safe YAML dumper
            return createSuccessResponse(totalPrice);
            
        } catch (IllegalArgumentException e) {
            // Return generic error without exposing internal details
            return createErrorResponse("Invalid input");
        } catch (Exception e) {
            // Log detailed error internally (not shown here)
            // Return generic error to client
            return createErrorResponse("Processing error");
        }
    }
    
    /**
     * Creates a safe YAML success response
     */
    private static String createSuccessResponse(double totalPrice) {
        DumperOptions options = new DumperOptions();
        options.setDefaultFlowStyle(DumperOptions.FlowStyle.BLOCK);
        Yaml yaml = new Yaml(options);
        
        Map<String, Object> response = new HashMap<>();
        response.put("status", "success");
        response.put("total_price", String.format("%.2f", totalPrice));
        
        return yaml.dump(response);
    }
    
    /**
     * Creates a safe YAML error response without leaking details
     */
    private static String createErrorResponse(String message) {
        DumperOptions options = new DumperOptions();
        options.setDefaultFlowStyle(DumperOptions.FlowStyle.BLOCK);
        Yaml yaml = new Yaml(options);
        
        Map<String, String> response = new HashMap<>();
        response.put("status", "error");
        response.put("message", message);
        
        return yaml.dump(response);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid input");
        String payload1 = "product_code: PROD001\\nquantity: 5";
        System.out.println(calculatePrice(payload1));
        
        System.out.println("\\nTest Case 2: Different product");
        String payload2 = "product_code: PROD003\\nquantity: 10";
        System.out.println(calculatePrice(payload2));
        
        System.out.println("\\nTest Case 3: Invalid product code");
        String payload3 = "product_code: INVALID\\nquantity: 5";
        System.out.println(calculatePrice(payload3));
        
        System.out.println("\\nTest Case 4: Negative quantity");
        String payload4 = "product_code: PROD002\\nquantity: -5";
        System.out.println(calculatePrice(payload4));
        
        System.out.println("\\nTest Case 5: Missing quantity");
        String payload5 = "product_code: PROD001";
        System.out.println(calculatePrice(payload5));
    }
}
