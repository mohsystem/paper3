
import java.sql.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task137 {
    private static final Pattern PRODUCT_ID_PATTERN = Pattern.compile("^[A-Za-z0-9_-]{1,50}$");
    private static final int MAX_INPUT_LENGTH = 50;
    
    public static class ProductDetails {
        private final String id;
        private final String name;
        private final double price;
        private final String description;
        
        public ProductDetails(String id, String name, double price, String description) {
            this.id = id;
            this.name = name;
            this.price = price;
            this.description = description;
        }
        
        @Override
        public String toString() {
            return String.format("Product[id=%s, name=%s, price=%.2f, description=%s]", 
                id, name, price, description);
        }
    }
    
    public static String validateProductId(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Product ID cannot be null");
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException("Product ID exceeds maximum length");
        }
        
        if (!PRODUCT_ID_PATTERN.matcher(input).matches()) {
            throw new IllegalArgumentException("Product ID contains invalid characters");
        }
        
        return input;
    }
    
    public static ProductDetails queryProduct(String productId, Connection conn) throws SQLException {
        String validatedId = validateProductId(productId);
        
        String query = "SELECT id, name, price, description FROM products WHERE id = ?";
        
        try (PreparedStatement stmt = conn.prepareStatement(query)) {
            stmt.setString(1, validatedId);
            
            try (ResultSet rs = stmt.executeQuery()) {
                if (rs.next()) {
                    return new ProductDetails(
                        rs.getString("id"),
                        rs.getString("name"),
                        rs.getDouble("price"),
                        rs.getString("description")
                    );
                }
            }
        }
        
        return null;
    }
    
    private static Connection createInMemoryDatabase() throws SQLException {
        Connection conn = DriverManager.getConnection("jdbc:h2:mem:testdb;DB_CLOSE_DELAY=-1");
        
        try (Statement stmt = conn.createStatement()) {
            stmt.execute("CREATE TABLE IF NOT EXISTS products (" +
                "id VARCHAR(50) PRIMARY KEY, " +
                "name VARCHAR(255) NOT NULL, " +
                "price DOUBLE NOT NULL, " +
                "description TEXT)");
            
            stmt.execute("INSERT INTO products VALUES ('P001', 'Laptop', 999.99, 'High-performance laptop')");
            stmt.execute("INSERT INTO products VALUES ('P002', 'Mouse', 29.99, 'Wireless mouse')");
            stmt.execute("INSERT INTO products VALUES ('P003', 'Keyboard', 79.99, 'Mechanical keyboard')");
            stmt.execute("INSERT INTO products VALUES ('P004', 'Monitor', 299.99, '27-inch monitor')");
            stmt.execute("INSERT INTO products VALUES ('P005', 'Headphones', 149.99, 'Noise-canceling headphones')");
        }
        
        return conn;
    }
    
    public static void main(String[] args) {
        try (Connection conn = createInMemoryDatabase()) {
            System.out.println("Test Case 1: Valid product ID");
            ProductDetails product1 = queryProduct("P001", conn);
            System.out.println(product1 != null ? product1 : "Product not found");
            
            System.out.println("\\nTest Case 2: Another valid product ID");
            ProductDetails product2 = queryProduct("P003", conn);
            System.out.println(product2 != null ? product2 : "Product not found");
            
            System.out.println("\\nTest Case 3: Non-existent product");
            ProductDetails product3 = queryProduct("P999", conn);
            System.out.println(product3 != null ? product3 : "Product not found");
            
            System.out.println("\\nTest Case 4: Invalid characters (SQL injection attempt)");
            try {
                ProductDetails product4 = queryProduct("P001' OR '1'='1", conn);
                System.out.println(product4 != null ? product4 : "Product not found");
            } catch (IllegalArgumentException e) {
                System.out.println("Validation error: " + e.getMessage());
            }
            
            System.out.println("\\nTest Case 5: Input too long");
            try {
                String longId = "P".repeat(100);
                ProductDetails product5 = queryProduct(longId, conn);
                System.out.println(product5 != null ? product5 : "Product not found");
            } catch (IllegalArgumentException e) {
                System.out.println("Validation error: " + e.getMessage());
            }
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
    }
}
