
import java.sql.*;
import java.util.Scanner;

public class Lse146 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/trading_db";
    private static final String DB_USER = "db_user";
    private static final String DB_PASSWORD = "db_password";
    
    public static boolean processBuyOrder(String username, String stockName, int stockQuantity) {
        // Input validation
        if (username == null || username.trim().isEmpty()) {
            System.out.println("Error: Username cannot be empty");
            return false;
        }
        
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("Error: Stock name cannot be empty");
            return false;
        }
        
        if (stockQuantity <= 0) {
            System.out.println("Error: Stock quantity must be positive");
            return false;
        }
        
        // Sanitize inputs
        username = username.trim();
        stockName = stockName.trim().replaceAll("[^a-zA-Z0-9]", "");
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Use parameterized query to prevent SQL injection
            String sql = "INSERT INTO buy_orders (username, stock_name, stock_quantity, order_date) VALUES (?, ?, ?, NOW())";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, stockName);
            pstmt.setInt(3, stockQuantity);
            
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                System.out.println("Buy order placed successfully!");
                System.out.println("Redirecting to profile page...");
                return true;
            }
            
        } catch (SQLException e) {
            System.out.println("Database error: " + e.getMessage());
            return false;
        } finally {
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.out.println("Error closing resources: " + e.getMessage());
            }
        }
        
        return false;
    }
    
    public static void main(String[] args) {
        System.out.println("Testing Buy Order Processing System\\n");
        
        // Test case 1: Valid order
        System.out.println("Test 1: Valid buy order");
        processBuyOrder("john_doe", "AAPL", 100);
        System.out.println();
        
        // Test case 2: Another valid order
        System.out.println("Test 2: Valid buy order for different stock");
        processBuyOrder("jane_smith", "GOOGL", 50);
        System.out.println();
        
        // Test case 3: Invalid - empty username
        System.out.println("Test 3: Empty username (should fail)");
        processBuyOrder("", "TSLA", 75);
        System.out.println();
        
        // Test case 4: Invalid - negative quantity
        System.out.println("Test 4: Negative quantity (should fail)");
        processBuyOrder("bob_jones", "MSFT", -10);
        System.out.println();
        
        // Test case 5: Invalid - empty stock name
        System.out.println("Test 5: Empty stock name (should fail)");
        processBuyOrder("alice_wilson", "", 200);
    }
}
