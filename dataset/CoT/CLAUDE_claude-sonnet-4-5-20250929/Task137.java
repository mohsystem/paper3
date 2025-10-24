
import java.sql.*;
import java.util.Scanner;
import java.util.regex.Pattern;

public class Task137 {
    private static final String DB_URL = "jdbc:sqlite:products.db";
    private static final Pattern PRODUCT_ID_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]{1,50}$");
    
    public static void main(String[] args) {
        Task137 task = new Task137();
        
        // Initialize database with sample data
        task.initializeDatabase();
        
        // Test cases
        System.out.println("Test Case 1: Valid product ID");
        task.getProductDetails("PROD001");
        
        System.out.println("\\nTest Case 2: Valid product ID");
        task.getProductDetails("PROD002");
        
        System.out.println("\\nTest Case 3: Non-existent product");
        task.getProductDetails("PROD999");
        
        System.out.println("\\nTest Case 4: Invalid input (SQL injection attempt)");
        task.getProductDetails("PROD001' OR '1'='1");
        
        System.out.println("\\nTest Case 5: Empty input");
        task.getProductDetails("");
    }
    
    public void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("DROP TABLE IF EXISTS products");
            stmt.execute("CREATE TABLE products (" +
                        "id TEXT PRIMARY KEY, " +
                        "name TEXT NOT NULL, " +
                        "description TEXT, " +
                        "price REAL NOT NULL, " +
                        "stock INTEGER NOT NULL)");
            
            String insertSQL = "INSERT INTO products (id, name, description, price, stock) VALUES (?, ?, ?, ?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
                String[][] products = {
                    {"PROD001", "Laptop", "High-performance laptop", "999.99", "50"},
                    {"PROD002", "Mouse", "Wireless mouse", "29.99", "200"},
                    {"PROD003", "Keyboard", "Mechanical keyboard", "79.99", "150"},
                    {"PROD004", "Monitor", "27-inch 4K monitor", "399.99", "75"},
                    {"PROD005", "Headphones", "Noise-cancelling headphones", "199.99", "100"}
                };
                
                for (String[] product : products) {
                    pstmt.setString(1, product[0]);
                    pstmt.setString(2, product[1]);
                    pstmt.setString(3, product[2]);
                    pstmt.setDouble(4, Double.parseDouble(product[3]));
                    pstmt.setInt(5, Integer.parseInt(product[4]));
                    pstmt.executeUpdate();
                }
            }
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public void getProductDetails(String productId) {
        if (!validateInput(productId)) {
            System.out.println("Invalid product ID format. Please use alphanumeric characters, hyphens, or underscores only.");
            return;
        }
        
        String query = "SELECT id, name, description, price, stock FROM products WHERE id = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, productId);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    System.out.println("Product Found:");
                    System.out.println("ID: " + rs.getString("id"));
                    System.out.println("Name: " + rs.getString("name"));
                    System.out.println("Description: " + rs.getString("description"));
                    System.out.println("Price: $" + rs.getDouble("price"));
                    System.out.println("Stock: " + rs.getInt("stock"));
                } else {
                    System.out.println("Product not found with ID: " + productId);
                }
            }
        } catch (SQLException e) {
            System.err.println("Database query error: " + e.getMessage());
        }
    }
    
    private boolean validateInput(String input) {
        if (input == null || input.trim().isEmpty()) {
            return false;
        }
        return PRODUCT_ID_PATTERN.matcher(input.trim()).matches();
    }
}
