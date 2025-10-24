import java.util.HashMap;
import java.util.Map;

public class Task137 {

    // A private nested class to represent a product.
    private static class Product {
        private final String name;
        private final double price;

        public Product(String name, double price) {
            this.name = name;
            this.price = price;
        }

        @Override
        public String toString() {
            return String.format("Name: %s, Price: $%.2f", name, price);
        }
    }

    // A static map to act as our in-memory product table.
    // Using 'final' ensures the map reference cannot be changed.
    private static final Map<Integer, Product> productTable = new HashMap<>();

    // Static initializer block to populate the product table.
    static {
        productTable.put(101, new Product("Laptop", 1200.50));
        productTable.put(102, new Product("Mouse", 25.00));
        productTable.put(103, new Product("Keyboard", 75.75));
        productTable.put(104, new Product("Monitor", 300.00));
        productTable.put(105, new Product("Webcam", 50.25));
    }

    /**
     * Inquires the product table for a product's details based on its ID.
     *
     * @param productId The ID of the product to look up.
     * @return A string containing the product details or a "not found" message.
     */
    public static String getProductDetails(int productId) {
        // Use containsKey for a clear check, preventing potential null pointer issues
        // if the map were allowed to store null values.
        if (productTable.containsKey(productId)) {
            Product product = productTable.get(productId);
            // Defensive check in case a null product was somehow inserted.
            if (product != null) {
                return String.format("Product ID: %d, %s", productId, product.toString());
            }
        }
        return "Product with ID " + productId + " not found.";
    }

    public static void main(String[] args) {
        System.out.println("--- Product Inquiry System ---");

        // Test cases
        int[] testCases = {101, 104, 105, 999, -1};

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": Searching for product ID " + testCases[i]);
            String result = getProductDetails(testCases[i]);
            System.out.println("Result: " + result);
            System.out.println();
        }
    }
}