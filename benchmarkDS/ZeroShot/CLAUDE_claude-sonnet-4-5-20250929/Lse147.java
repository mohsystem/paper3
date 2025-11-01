
import java.sql.*;
import java.util.Scanner;

public class Lse147 {
    private static final String DB_URL = "jdbc:sqlite:stocks.db";
    
    // Initialize database and create table if not exists
    public static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String createTableSQL = "CREATE TABLE IF NOT EXISTS orders (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                    "stock_name TEXT NOT NULL, " +
                    "quantity INTEGER NOT NULL, " +
                    "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(createTableSQL);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    // Secure method to insert buy order using prepared statements
    public static boolean buyOrder(String stockName, int quantity) {
        // Input validation
        if (stockName == null || stockName.trim().isEmpty()) {
            System.err.println("Error: Stock name cannot be empty");
            return false;
        }
        
        if (quantity <= 0) {
            System.err.println("Error: Quantity must be positive");
            return false;
        }
        
        // Sanitize stock name (allow only alphanumeric and common stock symbols)
        if (!stockName.matches("^[a-zA-Z0-9._-]+$")) {
            System.err.println("Error: Invalid stock name format");
            return false;
        }
        
        String sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, stockName.trim());
            pstmt.setInt(2, quantity);
            pstmt.executeUpdate();
            
            System.out.println("Order placed successfully!");
            return true;
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    // Method to view all stock orders
    public static void stockView() {
        String sql = "SELECT id, stock_name, quantity, order_date FROM orders ORDER BY order_date DESC";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            
            System.out.println("\\n=== Stock Orders ===");
            System.out.printf("%-5s %-15s %-10s %-20s%n", "ID", "Stock Name", "Quantity", "Order Date");
            System.out.println("-".repeat(55));
            
            while (rs.next()) {
                System.out.printf("%-5d %-15s %-10d %-20s%n",
                        rs.getInt("id"),
                        rs.getString("stock_name"),
                        rs.getInt("quantity"),
                        rs.getString("order_date"));
            }
            
        } catch (SQLException e) {
            System.err.println("Error viewing orders: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        // Test cases
        System.out.println("Running test cases...\\n");
        
        // Test case 1: Valid order
        System.out.println("Test 1: Valid order");
        buyOrder("AAPL", 100);
        
        // Test case 2: Valid order with different stock
        System.out.println("\\nTest 2: Another valid order");
        buyOrder("GOOGL", 50);
        
        // Test case 3: Invalid - empty stock name
        System.out.println("\\nTest 3: Empty stock name");
        buyOrder("", 25);
        
        // Test case 4: Invalid - negative quantity
        System.out.println("\\nTest 4: Negative quantity");
        buyOrder("MSFT", -10);
        
        // Test case 5: Invalid - special characters in stock name
        System.out.println("\\nTest 5: Invalid stock name with SQL injection attempt");
        buyOrder("TSLA'; DROP TABLE orders; --", 75);
        
        // Display all orders
        stockView();
    }
}
