import java.util.Map;
import java.util.HashMap;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.error.YAMLException;
import java.util.Collections;

// To run this code, you need the SnakeYAML library.
// Maven dependency:
// <dependency>
//     <groupId>org.yaml</groupId>
//     <artifactId>snakeyaml</artifactId>
//     <version>2.0</version>
// </dependency>

public class Lse90 {

    private static final Map<String, Double> productPrices;

    static {
        Map<String, Double> prices = new HashMap<>();
        prices.put("PROD001", 10.50);
        prices.put("PROD002", 25.00);
        prices.put("PROD003", 5.75);
        prices.put("PROD004", 150.00);
        productPrices = Collections.unmodifiableMap(prices);
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
     * Simulates a web route that takes a YAML payload, calculates the total price,
     * and returns the result as a YAML payload.
     * This implementation uses SnakeYAML's SafeConstructor to prevent arbitrary
     * object deserialization, which is a critical security measure.
     *
     * @param yamlPayload A string containing the YAML request.
     * @return A YAML string with the total price or an error message.
     */
    public static String calculate_price(String yamlPayload) {
        // Use SafeConstructor or default Yaml() for safe parsing.
        Yaml yaml = new Yaml(new SafeConstructor());
        Yaml outputYaml = new Yaml();
        Map<String, Object> result = new HashMap<>();

        try {
            Map<String, Object> data = yaml.load(yamlPayload);

            if (data == null || !data.containsKey("product_code") || !data.containsKey("quantity")) {
                result.put("error", "Invalid payload: 'product_code' and 'quantity' are required.");
                return outputYaml.dump(result);
            }

            String productCode = (String) data.get("product_code");
            int quantity = (Integer) data.get("quantity");
            
            if (quantity <= 0) {
                result.put("error", "Quantity must be positive.");
                return outputYaml.dump(result);
            }

            double price = get_price_for_product_code(productCode);

            if (price < 0) {
                result.put("error", "Product code not found.");
            } else {
                double totalPrice = price * quantity;
                result.put("total_price", totalPrice);
            }

        } catch (YAMLException | ClassCastException e) {
            result.put("error", "Failed to parse YAML payload or invalid data types.");
        } catch (Exception e) {
            result.put("error", "An unexpected error occurred.");
        }

        return outputYaml.dump(result);
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Valid request
            "product_code: PROD001\nquantity: 10",
            // Test Case 2: Another valid request
            "product_code: PROD004\nquantity: 2",
            // Test Case 3: Product code not found
            "product_code: PROD999\nquantity: 5",
            // Test Case 4: Invalid payload (missing quantity)
            "product_code: PROD002",
            // Test Case 5: Malformed YAML / Invalid data type
            "product_code: PROD003\nquantity: \"three\""
        };
        
        System.out.println("--- Running Java Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input:\n---\n" + testCases[i] + "\n---");
            String result = calculate_price(testCases[i]);
            System.out.println("Output:\n" + result);
        }
    }
}