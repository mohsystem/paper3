
import java.sql.*;
import java.util.*;

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
            System.out.println("Error initializing database: " + e.getMessage());
        }
    }
    
    // Insert buy order into database
    public static boolean buyOrder(String stockName, int quantity) {
        if (stockName == null || stockName.trim().isEmpty() || quantity <= 0) {
            return false;
        }
        
        String insertSQL = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
            pstmt.setString(1, stockName);
            pstmt.setInt(2, quantity);
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            System.out.println("Error inserting order: " + e.getMessage());
            return false;
        }
    }
    
    // View all stock orders
    public static List<String> stockView() {
        List<String> orders = new ArrayList<>();
        String selectSQL = "SELECT * FROM orders ORDER BY order_date DESC";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(selectSQL)) {
            
            while (rs.next()) {
                String order = String.format("ID: %d, Stock: %s, Quantity: %d, Date: %s",
                        rs.getInt("id"),
                        rs.getString("stock_name"),
                        rs.getInt("quantity"),
                        rs.getString("order_date"));
                orders.add(order);
            }
        } catch (SQLException e) {
            System.out.println("Error viewing orders: " + e.getMessage());
        }
        
        return orders;
    }
    
    public static void main(String[] args) {
        // Initialize database
        initializeDatabase();
        
        // Test case 1: Valid order
        System.out.println("Test 1: Adding AAPL with quantity 10");
        boolean result1 = buyOrder("AAPL", 10);
        System.out.println("Result: " + (result1 ? "Success" : "Failed"));
        
        // Test case 2: Valid order
        System.out.println("\\nTest 2: Adding GOOGL with quantity 5");
        boolean result2 = buyOrder("GOOGL", 5);
        System.out.println("Result: " + (result2 ? "Success" : "Failed"));
        
        // Test case 3: Invalid order (empty stock name)
        System.out.println("\\nTest 3: Adding empty stock name with quantity 15");
        boolean result3 = buyOrder("", 15);
        System.out.println("Result: " + (result3 ? "Success" : "Failed"));
        
        // Test case 4: Invalid order (negative quantity)
        System.out.println("\\nTest 4: Adding MSFT with quantity -5");
        boolean result4 = buyOrder("MSFT", -5);
        System.out.println("Result: " + (result4 ? "Success" : "Failed"));
        
        // Test case 5: Valid order
        System.out.println("\\nTest 5: Adding TSLA with quantity 20");
        boolean result5 = buyOrder("TSLA", 20);
        System.out.println("Result: " + (result5 ? "Success" : "Failed"));
        
        // Display all orders
        System.out.println("\\n=== Stock View ===");
        List<String> orders = stockView();
        for (String order : orders) {
            System.out.println(order);
        }
    }
}
