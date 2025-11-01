import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.error.YAMLException;

import java.util.Map;
import java.util.HashMap;

/**
 * Note: This code requires the SnakeYAML library.
 * Add the following dependency to your project (e.g., in pom.xml for Maven):
 * <dependency>
 *     <groupId>org.yaml</groupId>
 *     <artifactId>snakeyaml</artifactId>
 *     <version>2.0</version>
 * </dependency>
 */
public class Lse88 {

    private static final Map<String, Double> PRODUCT_PRICES = new HashMap<>();
    private static final int MAX_QUANTITY = 1_000_000;

    static {
        PRODUCT_PRICES.put("PROD_A", 19.99);
        PRODUCT_PRICES.put("PROD_B", 250.00);
        PRODUCT_PRICES.put("PROD_C", 0.50);
    }

    /**
     * Parses a YAML payload, calculates the total price, and returns it as a string.
     *
     * @param yamlPayload A string containing the YAML payload with 'product_code' and 'quantity'.
     * @return A string representing the total price formatted to two decimal places, or an error message.
     */
    public String calculate_price(String yamlPayload) {
        if (yamlPayload == null || yamlPayload.trim().isEmpty()) {
            return "Error: YAML payload cannot be null or empty.";
        }

        // Use SafeConstructor to prevent deserialization of arbitrary Java objects.
        Yaml yaml = new Yaml(new SafeConstructor());
        Map<String, Object> data;
        try {
            data = yaml.load(yamlPayload);
            if (data == null) {
                 return "Error: Invalid or empty YAML payload.";
            }
        } catch (YAMLException e) {
            // Log the exception in a real application, but return a generic message.
            return "Error: Failed to parse YAML.";
        }

        // Validate product_code
        Object productCodeObj = data.get("product_code");
        if (!(productCodeObj instanceof String)) {
            return "Error: Missing or invalid 'product_code'. Must be a string.";
        }
        String productCode = (String) productCodeObj;
        if (productCode.trim().isEmpty()){
            return "Error: 'product_code' cannot be empty.";
        }

        // Validate quantity
        Object quantityObj = data.get("quantity");
        if (!(quantityObj instanceof Integer)) {
            return "Error: Missing or invalid 'quantity'. Must be an integer.";
        }
        int quantity = (Integer) quantityObj;

        // Business logic validation
        if (!PRODUCT_PRICES.containsKey(productCode)) {
            return "Error: Product not found.";
        }
        if (quantity < 0) {
            return "Error: Quantity cannot be negative.";
        }
        if (quantity > MAX_QUANTITY) {
            return "Error: Quantity exceeds maximum limit of " + MAX_QUANTITY;
        }

        double price = PRODUCT_PRICES.get(productCode);
        double totalPrice = price * quantity;

        return String.format("%.2f", totalPrice);
    }
    
    /**
     * Main method with test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        Lse88 calculator = new Lse88();
        
        System.out.println("--- Test Cases ---");

        // Test Case 1: Valid input
        String test1 = "product_code: 'PROD_A'\nquantity: 10";
        System.out.println("Test 1 Input:\n" + test1);
        System.out.println("Test 1 Output: " + calculator.calculate_price(test1));
        System.out.println();

        // Test Case 2: Invalid YAML syntax
        String test2 = "product_code: 'PROD_B\nquantity: 5";
        System.out.println("Test 2 Input:\n" + test2);
        System.out.println("Test 2 Output: " + calculator.calculate_price(test2));
        System.out.println();

        // Test Case 3: Unknown product code
        String test3 = "product_code: 'PROD_X'\nquantity: 100";
        System.out.println("Test 3 Input:\n" + test3);
        System.out.println("Test 3 Output: " + calculator.calculate_price(test3));
        System.out.println();

        // Test Case 4: Negative quantity
        String test4 = "product_code: 'PROD_C'\nquantity: -5";
        System.out.println("Test 4 Input:\n" + test4);
        System.out.println("Test 4 Output: " + calculator.calculate_price(test4));
        System.out.println();

        // Test Case 5: Quantity as a string (invalid type)
        String test5 = "product_code: 'PROD_A'\nquantity: '20'";
        System.out.println("Test 5 Input:\n" + test5);
        System.out.println("Test 5 Output: " + calculator.calculate_price(test5));
        System.out.println();
    }
}