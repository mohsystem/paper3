
import java.sql.*;
import java.util.Scanner;
import java.util.regex.Pattern;

public class Lse147 {
    private static final String DB_URL = "jdbc:sqlite:stocks.db";
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]{1,50}$");
    
    public static void main(String[] args) {
        // Initialize database
        initializeDatabase();
        
        // Test cases
        System.out.println("Test Case 1:");
        buyOrder("AAPL", 10);
        
        System.out.println("\\nTest Case 2:");
        buyOrder("GOOGL", 5);
        
        System.out.println("\\nTest Case 3:");
        buyOrder("MSFT", 15);
        
        System.out.println("\\nTest Case 4 (Invalid - negative quantity):");
        buyOrder("TSLA", -5);
        
        System.out.println("\\nTest Case 5 (Invalid - SQL injection attempt):");
        buyOrder("AAPL'; DROP TABLE orders;--", 10);
        
        // Display all orders
        stockView();
    }
    
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS orders (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "stock_name TEXT NOT NULL," +
                        "quantity INTEGER NOT NULL," +
                        "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error");
        }
    }
    
    public static boolean buyOrder(String stockName, int quantity) {
        // Input validation
        if (!validateStockName(stockName)) {
            System.out.println("Error: Invalid stock name format");
            return false;
        }
        
        if (!validateQuantity(quantity)) {
            System.out.println("Error: Quantity must be positive");
            return false;
        }
        
        // Insert into database using parameterized query
        String sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, stockName);
            pstmt.setInt(2, quantity);
            pstmt.executeUpdate();
            
            System.out.println("Order placed successfully: " + stockName + " x " + quantity);
            return true;
            
        } catch (SQLException e) {
            System.err.println("Error placing order");
            return false;
        }
    }
    
    private static boolean validateStockName(String stockName) {
        return stockName != null && STOCK_NAME_PATTERN.matcher(stockName).matches();
    }
    
    private static boolean validateQuantity(int quantity) {
        return quantity > 0;
    }
    
    public static void stockView() {
        System.out.println("\\n=== Stock View ===");
        String sql = "SELECT id, stock_name, quantity, order_date FROM orders";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            
            while (rs.next()) {
                System.out.printf("ID: %d, Stock: %s, Quantity: %d, Date: %s%n",
                    rs.getInt("id"),
                    rs.getString("stock_name"),
                    rs.getInt("quantity"),
                    rs.getString("order_date"));
            }
            
        } catch (SQLException e) {
            System.err.println("Error retrieving orders");
        }
    }
}
