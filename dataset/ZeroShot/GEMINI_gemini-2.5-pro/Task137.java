import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

class Product {
    private final String name;
    private final double price;
    private final String description;

    public Product(String name, double price, String description) {
        this.name = name;
        this.price = price;
        this.description = description;
    }

    @Override
    public String toString() {
        return String.format("Name: %s, Price: %.2f, Description: %s", name, price, description);
    }
}

public class Task137 {

    private static final Map<String, Product> productTable;

    static {
        Map<String, Product> aMap = new HashMap<>();
        aMap.put("PROD101", new Product("Laptop", 999.99, "High-performance laptop"));
        aMap.put("PROD102", new Product("Mouse", 25.50, "Ergonomic wireless mouse"));
        aMap.put("PROD103", new Product("Keyboard", 75.00, "Mechanical gaming keyboard"));
        aMap.put("PROD104", new Product("Monitor", 300.00, "27-inch 4K UHD monitor"));
        productTable = Collections.unmodifiableMap(aMap);
    }

    /**
     * Inquires the product table for product details in a secure manner.
     * This method is secure because:
     * 1. It validates input to reject null or empty strings.
     * 2. It uses a pre-defined, in-memory map for lookups, which prevents
     *    any form of injection attack (like SQL injection) as user input
     *    is never used to construct a query.
     * 3. It gracefully handles cases where the product is not found.
     *
     * @param productId The ID of the product to look up.
     * @return A string with product details or an error message.
     */
    public static String getProductDetails(String productId) {
        // 1. Input Validation: Check for null or empty/whitespace-only strings.
        if (productId == null || productId.trim().isEmpty()) {
            return "Error: Product ID cannot be null or empty.";
        }

        // 2. Data Lookup: Safely look up the product in the pre-defined map.
        Product product = productTable.get(productId);

        // 3. Result Handling: Return details or a 'not found' message.
        if (product != null) {
            return product.toString();
        } else {
            return "Product not found.";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Product Inquiry System ---");

        // Test Case 1: Valid product ID
        String test1 = "PROD101";
        System.out.println("Querying for ID: '" + test1 + "'");
        System.out.println("Result: " + getProductDetails(test1) + "\n");

        // Test Case 2: Another valid product ID
        String test2 = "PROD103";
        System.out.println("Querying for ID: '" + test2 + "'");
        System.out.println("Result: " + getProductDetails(test2) + "\n");

        // Test Case 3: Non-existent product ID
        String test3 = "PROD999";
        System.out.println("Querying for ID: '" + test3 + "'");
        System.out.println("Result: " + getProductDetails(test3) + "\n");

        // Test Case 4: Empty string input (whitespace)
        String test4 = " ";
        System.out.println("Querying for ID: '" + test4 + "'");
        System.out.println("Result: " + getProductDetails(test4) + "\n");

        // Test Case 5: Null input
        String test5 = null;
        System.out.println("Querying for ID: " + test5);
        System.out.println("Result: " + getProductDetails(test5) + "\n");
    }
}