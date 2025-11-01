
import java.sql.*;
import java.util.Scanner;
import java.util.regex.Pattern;

public class Lse146 {
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,30}$");
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\-\\\\.]{1,20}$");
    
    public static boolean validateUsername(String username) {
        return username != null && USERNAME_PATTERN.matcher(username).matches();
    }
    
    public static boolean validateStockName(String stockName) {
        return stockName != null && STOCK_NAME_PATTERN.matcher(stockName).matches();
    }
    
    public static boolean validateQuantity(int quantity) {
        return quantity > 0 && quantity <= 1000000;
    }
    
    public static String insertBuyOrder(String username, String stockName, int stockQuantity) {
        if (!validateUsername(username)) {
            return "Error: Invalid username format";
        }
        if (!validateStockName(stockName)) {
            return "Error: Invalid stock name format";
        }
        if (!validateQuantity(stockQuantity)) {
            return "Error: Invalid quantity (must be 1-1000000)";
        }
        
        String url = "jdbc:sqlite:stock_orders.db";
        String insertSQL = "INSERT INTO buy_orders (username, stock_name, quantity, order_date) VALUES (?, ?, ?, datetime('now'))";
        
        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
            
            pstmt.setString(1, username);
            pstmt.setString(2, stockName);
            pstmt.setInt(3, stockQuantity);
            
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                return "redirect:/profile?user=" + username;
            } else {
                return "Error: Failed to insert order";
            }
            
        } catch (SQLException e) {
            return "Error: Database error - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Stock Order System Test Cases ===\\n");
        
        // Test Case 1: Valid order
        System.out.println("Test 1 - Valid order:");
        System.out.println(insertBuyOrder("john_doe", "AAPL", 100));
        
        // Test Case 2: Another valid order
        System.out.println("\\nTest 2 - Valid order with different stock:");
        System.out.println(insertBuyOrder("jane_smith", "GOOGL", 50));
        
        // Test Case 3: Invalid username (special chars)
        System.out.println("\\nTest 3 - Invalid username:");
        System.out.println(insertBuyOrder("user@123", "MSFT", 75));
        
        // Test Case 4: Invalid stock name (too long special chars)
        System.out.println("\\nTest 4 - Invalid stock name:");
        System.out.println(insertBuyOrder("testuser", "STOCK'; DROP TABLE--", 25));
        
        // Test Case 5: Invalid quantity (negative)
        System.out.println("\\nTest 5 - Invalid quantity:");
        System.out.println(insertBuyOrder("alice", "TSLA", -10));
    }
}
