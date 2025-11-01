
import java.sql.*;
import java.util.Scanner;

public class Lse146 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/stock_db";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static boolean buyStock(String username, String stockName, int stockQuantity) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            String sql = "INSERT INTO buy_orders (username, stock_name, stock_quantity, order_date) VALUES (?, ?, ?, NOW())";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, stockName);
            pstmt.setInt(3, stockQuantity);
            
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                System.out.println("Buy order inserted successfully!");
                System.out.println("Redirecting to profile page...");
                return true;
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        } finally {
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
        return false;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        buyStock("john_doe", "AAPL", 10);
        
        System.out.println("\\nTest Case 2:");
        buyStock("jane_smith", "GOOGL", 5);
        
        System.out.println("\\nTest Case 3:");
        buyStock("bob_jones", "MSFT", 15);
        
        System.out.println("\\nTest Case 4:");
        buyStock("alice_williams", "TSLA", 20);
        
        System.out.println("\\nTest Case 5:");
        buyStock("charlie_brown", "AMZN", 8);
    }
}
