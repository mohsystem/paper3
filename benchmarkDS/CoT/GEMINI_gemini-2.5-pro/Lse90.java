import java.util.Map;
import java.util.HashMap;
import java.util.LinkedHashMap;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.error.YAMLException;

// Note: This code requires the SnakeYAML library.
// For Maven, add this to your pom.xml:
// <dependency>
//     <groupId>org.yaml</groupId>
//     <artifactId>snakeyaml</artifactId>
//     <version>2.0</version>
// </dependency>

public class Lse90 {

    private static final Map<String, Double> productPrices = new HashMap<>();

    static {
        productPrices.put("PROD123", 19.99);
        productPrices.put("PROD456", 25.50);
        productPrices.put("PROD789", 0.99);
        productPrices.put("PROD000", 150.00);
    }

    /**
     * Returns the price for a given product code.
     *
     * @param productCode The code of the product.
     * @return The price of the product, or -1.0 if not found.
     */
    public static double get_price_for_product_code(String productCode) {
        return productPrices.getOrDefault(productCode, -1.0);
    }

    /**
     * Simulates a web route that calculates the total price from a YAML payload.
     *
     * @param yamlPayload A string containing the YAML request.
     * @return A YAML string with the total price or an error message.
     */
    public static String calculate_price(String yamlPayload) {
        Yaml yaml = new Yaml();
        Map<String, Object> result = new LinkedHashMap<>(); // Use LinkedHashMap to preserve order

        try {
            Map<String, Object> data = yaml.load(yamlPayload);
            
            if (data == null) {
                 result.put("error", "Invalid or empty YAML payload");
                 return yaml.dump(result);
            }

            // Securely extract and validate product_code
            if (!data.containsKey("product_code") || !(data.get("product_code") instanceof String)) {
                result.put("error", "Missing or invalid 'product_code'");
                return yaml.dump(result);
            }
            String productCode = (String) data.get("product_code");

            // Securely extract and validate quantity
            if (!data.containsKey("quantity") || !(data.get("quantity") instanceof Integer)) {
                result.put("error", "Missing or invalid 'quantity'");
                return yaml.dump(result);
            }
            int quantity = (Integer) data.get("quantity");
            
            if (quantity <= 0) {
                result.put("error", "Quantity must be a positive integer");
                return yaml.dump(result);
            }

            // Get unit price
            double unitPrice = get_price_for_product_code(productCode);
            if (unitPrice < 0) {
                result.put("error", "Product code not found");
                return yaml.dump(result);
            }

            // Calculate total price
            double totalPrice = unitPrice * quantity;
            result.put("total_price", totalPrice);

        } catch (YAMLException e) {
            result.put("error", "Malformed YAML payload");
        } catch (ClassCastException e) {
            result.put("error", "Invalid data types in YAML payload");
        } catch (Exception e) {
            result.put("error", "An unexpected error occurred: " + e.getMessage());
        }

        return yaml.dump(result);
    }

    public static void main(String[] args) {
        // Test Case 1: Valid request
        String test1 = "product_code: PROD123\nquantity: 2";
        System.out.println("Test 1 Input:\n" + test1);
        System.out.println("Test 1 Output:\n" + calculate_price(test1));
        // Expected: total_price: 39.98

        // Test Case 2: Product code not found
        String test2 = "product_code: UNKNOWN\nquantity: 5";
        System.out.println("Test 2 Input:\n" + test2);
        System.out.println("Test 2 Output:\n" + calculate_price(test2));
        // Expected: error: Product code not found

        // Test Case 3: Malformed YAML (invalid syntax)
        String test3 = "product_code: PROD456\n  quantity: 1";
        System.out.println("Test 3 Input:\n" + test3);
        System.out.println("Test 3 Output:\n" + calculate_price(test3));
        // Expected: error: Malformed YAML payload

        // Test Case 4: Invalid quantity (not an integer)
        String test4 = "product_code: PROD789\nquantity: 'three'";
        System.out.println("Test 4 Input:\n" + test4);
        System.out.println("Test 4 Output:\n" + calculate_price(test4));
        // Expected: error: Missing or invalid 'quantity'

        // Test Case 5: Missing product_code key
        String test5 = "quantity: 10";
        System.out.println("Test 5 Input:\n" + test5);
        System.out.println("Test 5 Output:\n" + calculate_price(test5));
        // Expected: error: Missing or invalid 'product_code'
    }
}