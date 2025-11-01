import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
// SnakeYAML is a third-party library. To compile and run this code,
// you need to have the snakeyaml jar file in your classpath.
// For example:
// javac -cp snakeyaml-1.27.jar Lse88.java
// java -cp ".;snakeyaml-1.27.jar" Lse88
// Download from: https://repo1.maven.org/maven2/org/yaml/snakeyaml/1.27/snakeyaml-1.27.jar
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import org.yaml.snakeyaml.error.YAMLException;

public class Lse88 {

    private static final Map<String, Double> productPrices = new HashMap<>();

    static {
        productPrices.put("PROD1", 10.50);
        productPrices.put("PROD2", 25.00);
        productPrices.put("PROD3", 5.75);
    }

    /**
     * Calculates the total price based on a YAML payload.
     * This is a secure implementation that uses SnakeYAML's SafeConstructor
     * to prevent arbitrary code execution during deserialization.
     *
     * @param yamlPayload A string containing the YAML payload.
     * @return A string representing the total price or an error message.
     */
    public static String calculatePrice(String yamlPayload) {
        // Use SafeConstructor to prevent deserialization of arbitrary Java objects.
        // This is a critical security measure.
        Yaml yaml = new Yaml(new SafeConstructor());
        try {
            Map<String, Object> data = yaml.load(yamlPayload);
            
            if (data == null) {
                return "Error: YAML payload is empty or invalid.";
            }

            if (!data.containsKey("product_code") || !data.containsKey("quantity")) {
                return "Error: Missing 'product_code' or 'quantity' in YAML payload.";
            }

            Object productCodeObj = data.get("product_code");
            Object quantityObj = data.get("quantity");

            if (!(productCodeObj instanceof String)) {
                return "Error: 'product_code' must be a string.";
            }
            String productCode = (String) productCodeObj;

            if (!(quantityObj instanceof Number)) {
                return "Error: 'quantity' must be a number.";
            }
            int quantity = ((Number) quantityObj).intValue();

            if (quantity <= 0) {
                return "Error: Quantity must be positive.";
            }

            if (!productPrices.containsKey(productCode)) {
                return "Error: Product code not found.";
            }

            double price = productPrices.get(productCode);
            double totalPrice = price * quantity;

            return String.format("%.2f", totalPrice);

        } catch (YAMLException e) {
            return "Error: Invalid YAML format. " + e.getMessage();
        } catch (Exception e) {
            return "Error: An unexpected error occurred. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        List<String> testCases = new ArrayList<>();

        // Test Case 1: Valid input
        testCases.add("product_code: \"PROD1\"\nquantity: 10");
        
        // Test Case 2: Unknown product code
        testCases.add("product_code: \"PROD99\"\nquantity: 5");

        // Test Case 3: Malformed YAML (invalid syntax)
        testCases.add("product_code: \"PROD2\" quantity: 2");

        // Test Case 4: Missing 'quantity' key
        testCases.add("product_code: \"PROD3\"");

        // Test Case 5: Invalid quantity type
        testCases.add("product_code: \"PROD1\"\nquantity: \"five\"");

        for (int i = 0; i < testCases.size(); i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input:\n---\n" + testCases.get(i) + "\n---");
            String result = calculatePrice(testCases.get(i));
            System.out.println("Output: " + result);
            System.out.println();
        }
    }
}