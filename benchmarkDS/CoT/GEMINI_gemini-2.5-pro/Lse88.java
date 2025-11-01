import java.util.Map;
import java.util.HashMap;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.error.YAMLException;

// The main class name is Lse88 as per instructions.
// Note: This code requires the SnakeYAML library.
// You can add it to your project, e.g., using Maven:
// <dependency>
//     <groupId>org.yaml</groupId>
//     <artifactId>snakeyaml</artifactId>
//     <version>2.0</version>
// </dependency>
public class Lse88 {

    private static final Map<String, Double> PRODUCT_PRICES = new HashMap<>();

    static {
        PRODUCT_PRICES.put("PROD001", 10.50);
        PRODUCT_PRICES.put("PROD002", 25.00);
        PRODUCT_PRICES.put("PROD003", 5.75);
    }

    /**
     * Parses a YAML payload, calculates the total price, and returns it as a string.
     *
     * @param yamlPayload A string containing the YAML payload with product_code and quantity.
     * @return A string representing the total price or an error message.
     */
    public String calculate_price(String yamlPayload) {
        // Use SafeConstructor to prevent arbitrary code execution during deserialization.
        Yaml yaml = new Yaml(new SafeConstructor());
        try {
            Map<String, Object> data = yaml.load(yamlPayload);

            if (data == null) {
                return "Error: YAML payload is empty or invalid.";
            }

            // Securely extract and validate keys and types
            if (!data.containsKey("product_code") || !data.containsKey("quantity")) {
                return "Error: Missing 'product_code' or 'quantity' in payload.";
            }

            Object productCodeObj = data.get("product_code");
            Object quantityObj = data.get("quantity");

            if (!(productCodeObj instanceof String)) {
                return "Error: 'product_code' must be a string.";
            }
            if (!(quantityObj instanceof Integer)) {
                return "Error: 'quantity' must be an integer.";
            }

            String productCode = (String) productCodeObj;
            int quantity = (Integer) quantityObj;

            // Input validation
            if (quantity <= 0) {
                return "Error: Quantity must be a positive integer.";
            }

            if (!PRODUCT_PRICES.containsKey(productCode)) {
                return "Error: Product code not found.";
            }

            double price = PRODUCT_PRICES.get(productCode);
            double totalPrice = price * quantity;

            // Return formatted string
            return String.format("%.2f", totalPrice);

        } catch (YAMLException e) {
            return "Error: Invalid YAML format. " + e.getMessage();
        } catch (Exception e) {
            // Catch any other unexpected errors
            return "Error: An unexpected error occurred. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        Lse88 calculator = new Lse88();

        // Test cases
        String[] testCases = {
            // 1. Valid case
            "product_code: 'PROD002'\nquantity: 3",
            // 2. Product not found
            "product_code: 'PROD999'\nquantity: 1",
            // 3. Invalid YAML format (e.g., incorrect indentation)
            "product_code: 'PROD001'\n quantity: 5",
            // 4. Missing key ('quantity')
            "product_code: 'PROD003'",
            // 5. Invalid data type for quantity
            "product_code: 'PROD001'\nquantity: 'two'"
        };
        
        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.printf("Test Case %d:\nInput:\n---\n%s\n---\n", i + 1, testCases[i]);
            String result = calculator.calculate_price(testCases[i]);
            System.out.printf("Output: %s\n\n", result);
        }
    }
}