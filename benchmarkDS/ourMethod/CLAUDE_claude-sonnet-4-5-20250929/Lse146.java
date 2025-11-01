
import java.sql.*;
import java.util.Properties;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.regex.Pattern;

public class Lse146 {
    // Secure input validation for stock name (alphanumeric and common stock symbols)
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,10}$");
    // Maximum quantity to prevent integer overflow and unrealistic orders
    private static final int MAX_QUANTITY = 1000000;
    private static final int MIN_QUANTITY = 1;
    
    public static boolean insertBuyOrder(String username, String stockName, int stockQuantity) {
        // Input validation: check for null and empty strings
        if (username == null || username.trim().isEmpty()) {
            System.err.println("Invalid username");
            return false;
        }
        if (stockName == null || stockName.trim().isEmpty()) {
            System.err.println("Invalid stock name");
            return false;
        }
        
        // Validate username length and format (alphanumeric and underscore only)
        if (username.length() > 50 || !username.matches("^[A-Za-z0-9_]{3,50}$")) {
            System.err.println("Username must be 3-50 alphanumeric characters");
            return false;
        }
        
        // Validate stock name format to prevent injection
        if (!STOCK_NAME_PATTERN.matcher(stockName).matches()) {
            System.err.println("Invalid stock name format");
            return false;
        }
        
        // Validate quantity range to prevent negative values and overflow
        if (stockQuantity < MIN_QUANTITY || stockQuantity > MAX_QUANTITY) {
            System.err.println("Quantity must be between " + MIN_QUANTITY + " and " + MAX_QUANTITY);
            return false;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Load database credentials from secure configuration
            Properties props = new Properties();
            try (FileInputStream fis = new FileInputStream("db.properties")) {
                props.load(fis);
            }
            
            String dbUrl = props.getProperty("db.url");
            String dbUser = props.getProperty("db.user");
            String dbPassword = props.getProperty("db.password");
            
            // Validate configuration was loaded
            if (dbUrl == null || dbUser == null || dbPassword == null) {
                System.err.println("Database configuration incomplete");
                return false;
            }
            
            // Establish secure database connection with SSL
            Properties connProps = new Properties();
            connProps.setProperty("user", dbUser);
            connProps.setProperty("password", dbPassword);
            connProps.setProperty("useSSL", "true");
            connProps.setProperty("requireSSL", "true");
            
            conn = DriverManager.getConnection(dbUrl, connProps);
            
            // Use parameterized query to prevent SQL injection
            String sql = "INSERT INTO buy_orders (username, stock_name, quantity, order_date) VALUES (?, ?, ?, NOW())";
            pstmt = conn.prepareStatement(sql);
            
            // Set parameters with validated inputs
            pstmt.setString(1, username);
            pstmt.setString(2, stockName);
            pstmt.setInt(3, stockQuantity);
            
            // Execute the insert
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                System.out.println("Order inserted successfully");
                return true;
            } else {
                System.err.println("Failed to insert order");
                return false;
            }
            
        } catch (SQLException e) {
            // Log error without exposing sensitive details
            System.err.println("Database error occurred");
            return false;
        } catch (IOException e) {
            System.err.println("Configuration error");
            return false;
        } finally {
            // Clean up resources in reverse order
            if (pstmt != null) {
                try {
                    pstmt.close();
                } catch (SQLException e) {
                    System.err.println("Error closing statement");
                }
            }
            if (conn != null) {
                try {
                    conn.close();
                } catch (SQLException e) {
                    System.err.println("Error closing connection");
                }
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid input
        System.out.println("Test 1: " + insertBuyOrder("john_doe", "AAPL", 100));
        
        // Test case 2: Valid input with different stock
        System.out.println("Test 2: " + insertBuyOrder("jane_smith", "GOOGL", 50));
        
        // Test case 3: Invalid stock name (contains invalid characters)
        System.out.println("Test 3: " + insertBuyOrder("user123", "AAPL'; DROP TABLE--", 10));
        
        // Test case 4: Invalid quantity (negative)
        System.out.println("Test 4: " + insertBuyOrder("alice_w", "MSFT", -5));
        
        // Test case 5: Invalid quantity (exceeds maximum)
        System.out.println("Test 5: " + insertBuyOrder("bob_jones", "TSLA", 2000000));
    }
}
