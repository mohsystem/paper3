
import java.sql.*;
import java.util.Scanner;

public class Lse145 {
    private static final String DB_URL = "jdbc:sqlite:stocks.db";
    
    // Initialize database
    private static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS stocks " +
                        "(id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                        " stock_name TEXT NOT NULL, " +
                        " quantity INTEGER NOT NULL, " +
                        " purchase_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    // Helper function called after buying stock
    private static boolean buyFunction(String stockName) {
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("Invalid stock name");
            return false;
        }
        
        // Sanitize stock name
        stockName = stockName.trim().replaceAll("[^a-zA-Z0-9]", "");
        
        System.out.println("Processing purchase for stock: " + stockName);
        // Additional business logic can be added here
        return true;
    }
    
    // Main function to buy stock - secure implementation
    public static boolean buyStock(String stockName, int quantity) {
        // Input validation
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("Error: Stock name cannot be empty");
            return false;
        }
        
        if (quantity <= 0) {
            System.out.println("Error: Quantity must be positive");
            return false;
        }
        
        // Sanitize input
        stockName = stockName.trim().replaceAll("[^a-zA-Z0-9]", "");
        
        // Call buy_function
        if (!buyFunction(stockName)) {
            return false;
        }
        
        // Insert into database using parameterized query (prevents SQL injection)
        String sql = "INSERT INTO stocks(stock_name, quantity) VALUES(?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, stockName);
            pstmt.setInt(2, quantity);
            
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                System.out.println("Successfully purchased " + quantity + " shares of " + stockName);
                return true;
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
        
        return false;
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        initDatabase();
        
        System.out.println("=== Test Case 1: Valid purchase ===");
        buyStock("AAPL", 10);
        
        System.out.println("\\n=== Test Case 2: Another valid purchase ===");
        buyStock("GOOGL", 5);
        
        System.out.println("\\n=== Test Case 3: Invalid quantity (negative) ===");
        buyStock("MSFT", -5);
        
        System.out.println("\\n=== Test Case 4: Invalid quantity (zero) ===");
        buyStock("TSLA", 0);
        
        System.out.println("\\n=== Test Case 5: Empty stock name ===");
        buyStock("", 10);
    }
}
