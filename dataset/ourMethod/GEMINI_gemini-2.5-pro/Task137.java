import java.util.Map;
import java.util.HashMap;
import java.util.Objects;

public class Task137 {

    // Using a record for an immutable data carrier
    public record Product(String name, double price) {}

    private static final Map<String, Product> PRODUCT_TABLE;

    static {
        Map<String, Product> aMap = new HashMap<>();
        aMap.put("A123", new Product("Laptop", 999.99));
        aMap.put("B456", new Product("Mouse", 25.50));
        aMap.put("C789", new Product("Keyboard", 75.00));
        PRODUCT_TABLE = Map.copyOf(aMap); // Make it immutable
    }

    /**
     * Inquires the product table for product details.
     *
     * @param productId The ID of the product to look for. Must not be null or blank.
     * @return A string with product details or an error message.
     */
    public static String getProductDetails(String productId) {
        // Rule #3: Validate input. Treat all inputs as untrusted.
        if (productId == null || productId.trim().isEmpty()) {
            return "Error: Product ID cannot be null or empty.";
        }

        // Rule #1: Use safe parsing and formatting.
        Product product = PRODUCT_TABLE.get(productId);

        if (product != null) {
            return String.format("Product: %s, Price: $%.2f", product.name(), product.price());
        } else {
            // Sanitize productId before including it in the output message to prevent potential injection issues,
            // though in this simple case it's less critical. A simple length check can be a form of sanitization.
            String sanitizedId = productId.length() > 50 ? productId.substring(0, 50) + "..." : productId;
            sanitizedId = sanitizedId.replaceAll("[^a-zA-Z0-9-]", ""); // Basic sanitization
            return String.format("Product with ID '%s' not found.", sanitizedId);
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Product Inquiry System ---");

        String[] testCases = {
            "A123",       // Test Case 1: Valid, existing product ID
            "C789",       // Test Case 2: Another valid, existing product ID
            "X999",       // Test Case 3: Valid format, but non-existent ID
            "",           // Test Case 4: Invalid empty string
            null          // Test Case 5: Invalid null input
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.printf("Test Case %d (Input: \"%s\"):\n", i + 1, testCases[i]);
            String result = getProductDetails(testCases[i]);
            System.out.println("  Result: " + result);
            System.out.println();
        }
    }
}