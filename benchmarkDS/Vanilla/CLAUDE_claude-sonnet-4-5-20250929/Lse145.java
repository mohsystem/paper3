
import java.sql.*;
import java.util.*;

public class Lse145 {
    private static final String DB_URL = "jdbc:sqlite:stocks.db";
    
    // Simulates getting form data
    static class FormData {
        String stockName;
        int quantity;
        
        FormData(String stockName, int quantity) {
            this.stockName = stockName;
            this.quantity = quantity;
        }
    }
    
    // The buy_function that processes stock name
    public static void buy_function(String stockName) {
        System.out.println("Processing stock: " + stockName);
        // Additional stock processing logic can be added here
        System.out.println("Stock " + stockName + " processed successfully");
    }
    
    // The buy_stock function that handles the purchase
    public static boolean buy_stock(String stockName, int quantity) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Create database connection
            conn = DriverManager.getConnection(DB_URL);
            
            // Create table if not exists
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE IF NOT EXISTS stocks (id INTEGER PRIMARY KEY AUTOINCREMENT, stock_name TEXT, quantity INTEGER, purchase_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
            
            // Insert stock data into database
            String sql = "INSERT INTO stocks (stock_name, quantity) VALUES (?, ?)";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, stockName);
            pstmt.setInt(2, quantity);
            pstmt.executeUpdate();
            
            System.out.println("Stock inserted into database: " + stockName + ", Quantity: " + quantity);
            
            // Call buy_function with stock name
            buy_function(stockName);
            
            return true;
            
        } catch (SQLException e) {
            System.err.println("Error: " + e.getMessage());
            return false;
        } finally {
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing connection: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1 ===");
        buy_stock("AAPL", 10);
        
        System.out.println("\\n=== Test Case 2 ===");
        buy_stock("GOOGL", 5);
        
        System.out.println("\\n=== Test Case 3 ===");
        buy_stock("MSFT", 15);
        
        System.out.println("\\n=== Test Case 4 ===");
        buy_stock("TSLA", 8);
        
        System.out.println("\\n=== Test Case 5 ===");
        buy_stock("AMZN", 3);
    }
}
