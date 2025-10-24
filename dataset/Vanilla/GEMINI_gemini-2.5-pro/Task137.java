import java.util.HashMap;
import java.util.Map;

public class Task137 {

    // A simple class to represent a product
    static class Product {
        int id;
        String name;
        double price;
        int quantity;

        public Product(int id, String name, double price, int quantity) {
            this.id = id;
            this.name = name;
            this.price = price;
            this.quantity = quantity;
        }

        @Override
        public String toString() {
            return String.format("ID: %d, Name: %s, Price: %.2f, Quantity: %d", id, name, price, quantity);
        }
    }

    // Static product table (simulating a database)
    private static final Map<String, Product> productTable = new HashMap<>();

    static {
        productTable.put("Laptop", new Product(101, "Laptop", 1200.50, 10));
        productTable.put("Mouse", new Product(102, "Mouse", 25.00, 150));
        productTable.put("Keyboard", new Product(103, "Keyboard", 75.99, 80));
        productTable.put("Monitor", new Product(104, "Monitor", 300.00, 45));
        productTable.put("USB Hub", new Product(105, "USB Hub", 15.50, 200));
    }

    /**
     * Inquires the product table for a product's details by its name.
     * @param productName The name of the product to search for.
     * @return A string with the product details or a "not found" message.
     */
    public static String getProductDetails(String productName) {
        Product product = productTable.get(productName);
        if (product != null) {
            return product.toString();
        } else {
            return "Product not found.";
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] testCases = {"Laptop", "Monitor", "Mouse", "Webcam", "USB Hub"};

        System.out.println("--- Java Product Inquiry System ---");
        for (String testCase : testCases) {
            System.out.println("Querying for: " + testCase);
            String details = getProductDetails(testCase);
            System.out.println("Result: " + details);
            System.out.println();
        }
    }
}