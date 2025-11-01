import java.util.Map;
import java.util.HashMap;

public class Lse88 {

    private static final Map<String, Double> prices = new HashMap<>();

    static {
        prices.put("P101", 10.50);
        prices.put("P102", 25.00);
        prices.put("P103", 5.75);
    }

    /**
     * Parses a simple YAML string to calculate the total price of a product.
     *
     * @param yamlPayload A string containing product_code and quantity.
     * @return The total price formatted as a string, or an error message.
     */
    public static String calculate_price(String yamlPayload) {
        String productCode = null;
        Integer quantity = null;

        try {
            String[] lines = yamlPayload.trim().split("\\r?\\n");
            for (String line : lines) {
                String[] parts = line.split(":", 2);
                if (parts.length != 2) {
                     // This handles cases where a line doesn't have a colon
                     continue; 
                }
                String key = parts[0].trim();
                String value = parts[1].trim();

                if (key.equals("product_code")) {
                    productCode = value.replace("\"", ""); // Remove quotes
                } else if (key.equals("quantity")) {
                    quantity = Integer.parseInt(value);
                }
            }

            if (productCode == null || quantity == null) {
                return "Error: Invalid format or missing keys";
            }

            if (!prices.containsKey(productCode)) {
                return "Error: Product not found";
            }

            double unitPrice = prices.get(productCode);
            double totalPrice = unitPrice * quantity;

            return String.format("%.2f", totalPrice);

        } catch (NumberFormatException e) {
            return "Error: Invalid quantity";
        } catch (Exception e) {
            return "Error: An unexpected error occurred";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Standard valid case
            "product_code: \"P101\"\nquantity: 5",
            // Test Case 2: Another valid case (different order)
            "quantity: 10\nproduct_code: \"P103\"",
            // Test Case 3: Product not found
            "product_code: \"P999\"\nquantity: 2",
            // Test Case 4: Invalid quantity
            "product_code: \"P102\"\nquantity: abc",
            // Test Case 5: Malformed YAML (no colon on one line)
            "product_code \"P101\"\nquantity: 3"
        };

        System.out.println("Java Test Cases:");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input:\n---\n" + testCases[i] + "\n---");
            String result = calculate_price(testCases[i]);
            System.out.println("Output: " + result);
            System.out.println();
        }
    }
}