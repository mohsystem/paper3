
import java.util.*;

public class Task137 {
    static class Product {
        String id;
        String name;
        double price;
        String category;
        int stock;
        
        public Product(String id, String name, double price, String category, int stock) {
            this.id = id;
            this.name = name;
            this.price = price;
            this.category = category;
            this.stock = stock;
        }
        
        @Override
        public String toString() {
            return String.format("ID: %s, Name: %s, Price: $%.2f, Category: %s, Stock: %d", 
                               id, name, price, category, stock);
        }
    }
    
    static Map<String, Product> productTable = new HashMap<>();
    
    static {
        productTable.put("P001", new Product("P001", "Laptop", 999.99, "Electronics", 50));
        productTable.put("P002", new Product("P002", "Mouse", 29.99, "Electronics", 200));
        productTable.put("P003", new Product("P003", "Keyboard", 79.99, "Electronics", 150));
        productTable.put("P004", new Product("P004", "Monitor", 299.99, "Electronics", 75));
        productTable.put("P005", new Product("P005", "Desk Chair", 199.99, "Furniture", 30));
    }
    
    public static Product queryProduct(String productId) {
        return productTable.get(productId);
    }
    
    public static List<Product> queryProductsByCategory(String category) {
        List<Product> results = new ArrayList<>();
        for (Product product : productTable.values()) {
            if (product.category.equalsIgnoreCase(category)) {
                results.add(product);
            }
        }
        return results;
    }
    
    public static List<Product> queryProductsByPriceRange(double minPrice, double maxPrice) {
        List<Product> results = new ArrayList<>();
        for (Product product : productTable.values()) {
            if (product.price >= minPrice && product.price <= maxPrice) {
                results.add(product);
            }
        }
        return results;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Query by Product ID");
        Product product1 = queryProduct("P001");
        System.out.println(product1 != null ? product1 : "Product not found");
        System.out.println();
        
        System.out.println("Test Case 2: Query by Product ID (Non-existent)");
        Product product2 = queryProduct("P999");
        System.out.println(product2 != null ? product2 : "Product not found");
        System.out.println();
        
        System.out.println("Test Case 3: Query by Category");
        List<Product> electronics = queryProductsByCategory("Electronics");
        System.out.println("Electronics products: " + electronics.size());
        for (Product p : electronics) {
            System.out.println(p);
        }
        System.out.println();
        
        System.out.println("Test Case 4: Query by Price Range (50-100)");
        List<Product> midRange = queryProductsByPriceRange(50.0, 100.0);
        System.out.println("Products in range: " + midRange.size());
        for (Product p : midRange) {
            System.out.println(p);
        }
        System.out.println();
        
        System.out.println("Test Case 5: Query by Category (Furniture)");
        List<Product> furniture = queryProductsByCategory("Furniture");
        System.out.println("Furniture products: " + furniture.size());
        for (Product p : furniture) {
            System.out.println(p);
        }
    }
}
