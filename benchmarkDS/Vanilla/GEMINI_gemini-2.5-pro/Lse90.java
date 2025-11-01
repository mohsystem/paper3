import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse90 {

    private static final Map<String, Double> PRICE_MAP = new HashMap<>();
    static {
        PRICE_MAP.put("P101", 10.50);
        PRICE_MAP.put("P202", 25.00);
        PRICE_MAP.put("P303", 5.75);
    }

    /**
     * Returns the price for a given product code.
     * @param productCode The code of the product.
     * @return The price of the product, or 0.0 if not found.
     */
    public static double get_price_for_product_code(String productCode) {
        return PRICE_MAP.getOrDefault(productCode, 0.0);
    }

    /**
     * A very simple parser for a specific YAML format (e.g., "key: value").
     * @param yamlPayload The YAML string to parse.
     * @return A map of key-value pairs.
     */
    private static Map<String, String> parseSimpleYaml(String yamlPayload) {
        Map<String, String> data = new HashMap<>();
        Pattern pattern = Pattern.compile("(\\w+):\\s*(\\S+)");
        Matcher matcher = pattern.matcher(yamlPayload);
        while (matcher.find()) {
            data.put(matcher.group(1), matcher.group(2));
        }
        return data;
    }

    /**
     * A very simple generator for a YAML format.
     * @param data The map to convert to a YAML string.
     * @return The resulting YAML string.
     */
    private static String generateSimpleYaml(Map<String, Object> data) {
        StringBuilder sb = new StringBuilder();
        for (Map.Entry<String, Object> entry : data.entrySet()) {
            sb.append(entry.getKey()).append(": ").append(entry.getValue()).append("\n");
        }
        return sb.toString();
    }

    /**
     * Simulates a web route that calculates the total price from a YAML payload.
     * @param yamlPayload A string in a simple YAML format containing product_code and quantity.
     * @return A YAML string containing the total_price.
     */
    public static String calculate_price(String yamlPayload) {
        // Load YAML into a dictionary (Map)
        Map<String, String> data = parseSimpleYaml(yamlPayload);
        
        // Extract product code and quantity
        String productCode = data.get("product_code");
        int quantity = 0;
        if (data.containsKey("quantity")) {
            try {
                quantity = Integer.parseInt(data.get("quantity"));
            } catch (NumberFormatException e) {
                quantity = 0; // Default to 0 if quantity is not a valid number
            }
        }
        
        // Calculate the total price
        double pricePerUnit = get_price_for_product_code(productCode);
        double totalPrice = pricePerUnit * quantity;
        
        // Prepare the result map
        Map<String, Object> result = new HashMap<>();
        result.put("total_price", String.format("%.2f", totalPrice));
        
        // Return the total price as a YAML payload
        return generateSimpleYaml(result);
    }

    public static void main(String[] args) {
        String[] testCases = {
            "product_code: P101\nquantity: 5",    // Expected: 52.50
            "product_code: P202\nquantity: 1",    // Expected: 25.00
            "product_code: P303\nquantity: 100",  // Expected: 575.00
            "product_code: P999\nquantity: 10",   // Expected: 0.00 (product not found)
            "product_code: P101\nquantity: 0"     // Expected: 0.00
        };
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input YAML:\n---\n" + testCases[i] + "\n---");
            String outputYaml = calculate_price(testCases[i]);
            System.out.println("Output YAML:\n" + outputYaml);
        }
    }
}