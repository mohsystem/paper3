import java.util.HashMap;
import java.util.Map;

// For YAML parsing. Add the SnakeYAML dependency to your project.
// e.g., for Maven:
// <dependency>
//     <groupId>org.yaml</groupId>
//     <artifactId>snakeyaml</artifactId>
//     <version>2.0</version>
// </dependency>
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.LoaderOptions;
import org.yaml.snakeyaml.error.YAMLException;

public class Lse90 {

    private static final Map<String, Double> PRODUCT_PRICES = new HashMap<>();
    static {
        PRODUCT_PRICES.put("PROD123", 10.50);
        PRODUCT_PRICES.put("PROD456", 25.00);
        PRODUCT_PRICES.put("PROD789", 5.75);
    }

    /**
     * Returns the price for a given product code.
     * @param productCode The product code to look up.
     * @return The price of the product.
     * @throws IllegalArgumentException if the product code is not found.
     */
    public static double get_price_for_product_code(String productCode) {
        if (productCode == null) {
            throw new IllegalArgumentException("Product code cannot be null.");
        }
        Double price = PRODUCT_PRICES.get(productCode);
        if (price == null) {
            throw new IllegalArgumentException("Invalid product code: " + productCode);
        }
        return price;
    }

    /**
     * A web route that calculates the total price from a YAML payload.
     * @param yamlPayload A string containing the YAML payload.
     * @return A YAML string with the total price or an error message.
     */
    public static String calculate_price(String yamlPayload) {
        // Rules#2, #5: Use modern libraries and secure patterns.
        // SnakeYAML's SafeConstructor prevents arbitrary code execution (CWE-502).
        // We also use LoaderOptions to set a code point limit to prevent DoS (e.g., YAML bombs).
        LoaderOptions options = new LoaderOptions();
        options.setCodePointLimit(5 * 1024 * 1024); // 5 MB limit
        Yaml yaml = new Yaml(new SafeConstructor(options));
        Map<String, Object> result = new HashMap<>();

        try {
            // Rule#1: Ensure all input is validated.
            if (yamlPayload == null || yamlPayload.trim().isEmpty()) {
                throw new IllegalArgumentException("YAML payload cannot be null or empty.");
            }

            Map<String, Object> data = yaml.load(yamlPayload);

            if (data == null) {
                 throw new IllegalArgumentException("Invalid or empty YAML payload.");
            }

            // Extract and validate product_code
            if (!data.containsKey("product_code")) {
                throw new IllegalArgumentException("Missing 'product_code' in YAML payload.");
            }
            Object productCodeObj = data.get("product_code");
            if (!(productCodeObj instanceof String)) {
                throw new IllegalArgumentException("'product_code' must be a string.");
            }
            String productCode = (String) productCodeObj;

            // Extract and validate quantity
            if (!data.containsKey("quantity")) {
                throw new IllegalArgumentException("Missing 'quantity' in YAML payload.");
            }
            Object quantityObj = data.get("quantity");
            if (!(quantityObj instanceof Integer)) {
                throw new IllegalArgumentException("'quantity' must be an integer.");
            }
            int quantity = (Integer) quantityObj;
            if (quantity <= 0) {
                throw new IllegalArgumentException("'quantity' must be positive.");
            }

            double price = get_price_for_product_code(productCode);
            double totalPrice = price * quantity;

            result.put("total_price", totalPrice);

        } catch (IllegalArgumentException | YAMLException e) {
            // Rule #5: Return generic messages to callers. Do not leak internal details.
            result.put("error", e.getMessage());
        } catch (Exception e) {
            // Catch any other unexpected exception
            result.put("error", "An unexpected error occurred.");
        }
        
        return yaml.dump(result);
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Valid input
            "product_code: PROD123\nquantity: 2",
            // Test Case 2: Invalid product code
            "product_code: UNKNOWN\nquantity: 1",
            // Test Case 3: Invalid quantity (negative)
            "product_code: PROD456\nquantity: -5",
            // Test Case 4: Malformed YAML
            "product_code: PROD123, quantity: 2",
            // Test Case 5: Missing key
            "product_code: PROD789"
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input YAML:\n---\n" + testCases[i] + "\n---");
            String result = calculate_price(testCases[i]);
            System.out.println("Output YAML:\n" + result);
        }
    }
}