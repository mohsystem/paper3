
import java.sql.*;
import java.util.Properties;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.SecureRandom;
import javax.crypto.Cipher;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.util.Base64;

public class Lse145 {
    private static final int MAX_STOCK_NAME_LENGTH = 50;
    private static final int MAX_QUANTITY = 1000000;
    private static final int MIN_QUANTITY = 1;
    
    // Secure database connection with validation
    private static Connection getSecureConnection() throws SQLException {
        try {
            // Load database credentials from secure configuration file or environment variables
            // Never hardcode credentials
            String dbUrl = System.getenv("DB_URL");
            String dbUser = System.getenv("DB_USER");
            String dbPassword = System.getenv("DB_PASSWORD");
            
            // Validate environment variables are set
            if (dbUrl == null || dbUser == null || dbPassword == null) {
                throw new SQLException("Database credentials not properly configured");
            }
            
            // Use properties to configure secure connection
            Properties props = new Properties();
            props.setProperty("user", dbUser);
            props.setProperty("password", dbPassword);
            props.setProperty("useSSL", "true");
            props.setProperty("requireSSL", "true");
            props.setProperty("verifyServerCertificate", "true");
            
            return DriverManager.getConnection(dbUrl, props);
        } catch (Exception e) {
            // Log detailed error securely, return generic message to user
            System.err.println("Database connection failed");
            throw new SQLException("Unable to establish database connection");
        }
    }
    
    // Validates stock name input - prevents injection and path traversal
    private static boolean isValidStockName(String stockName) {
        if (stockName == null || stockName.trim().isEmpty()) {
            return false;
        }
        
        // Check length bounds
        if (stockName.length() > MAX_STOCK_NAME_LENGTH) {
            return false;
        }
        
        // Allow only alphanumeric characters, spaces, hyphens, and underscores
        // Prevents SQL injection and command injection
        if (!stockName.matches("^[a-zA-Z0-9\\\\s_-]+$")) {
            return false;
        }
        
        return true;
    }
    
    // Validates quantity input - prevents integer overflow and business logic errors
    private static boolean isValidQuantity(int quantity) {
        // Check range to prevent negative values and unreasonably large numbers
        return quantity >= MIN_QUANTITY && quantity <= MAX_QUANTITY;
    }
    
    // Buy function - processes stock name with validation
    private static String buyFunction(String stockName) {
        // Input validation - treat all input as untrusted
        if (!isValidStockName(stockName)) {
            return "Invalid stock name";
        }
        
        // Sanitize by trimming whitespace
        String sanitizedStockName = stockName.trim();
        
        // Business logic for buying stock
        // This could include additional validation, price checks, etc.
        return "Processing purchase for stock: " + sanitizedStockName;
    }
    
    // Main function to handle stock purchase - called when buy button is clicked
    public static String buyStock(String stockName, int quantity) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Input validation - fail early on invalid input
            if (!isValidStockName(stockName)) {
                return "Error: Invalid stock name format";
            }
            
            if (!isValidQuantity(quantity)) {
                return "Error: Invalid quantity. Must be between " + MIN_QUANTITY + " and " + MAX_QUANTITY;
            }
            
            // Sanitize inputs
            String sanitizedStockName = stockName.trim();
            
            // Call buy_function to process the stock name
            String buyResult = buyFunction(sanitizedStockName);
            
            // Get secure database connection
            conn = getSecureConnection();
            
            // Use PreparedStatement to prevent SQL injection
            // Never concatenate user input into SQL queries
            String sql = "INSERT INTO stock_purchases (stock_name, quantity, purchase_date) VALUES (?, ?, ?)";
            pstmt = conn.prepareStatement(sql);
            
            // Set parameters with type safety
            pstmt.setString(1, sanitizedStockName);
            pstmt.setInt(2, quantity);
            pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
            
            // Execute insert
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                return "Stock purchase successful: " + buyResult;
            } else {
                return "Stock purchase failed";
            }
            
        } catch (SQLException e) {
            // Log detailed error to secure log, return generic message
            System.err.println("Database error during stock purchase: " + e.getMessage());
            return "Error: Unable to complete purchase";
        } finally {
            // Clean up resources - prevent resource leaks
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing database resources");
            }
        }
    }
    
    // Test cases
    public static void main(String[] args) {
        System.out.println("Test Case 1 - Valid stock purchase:");
        System.out.println(buyStock("AAPL", 10));
        
        System.out.println("\\nTest Case 2 - Valid stock with hyphen:");
        System.out.println(buyStock("TECH-CORP", 50));
        
        System.out.println("\\nTest Case 3 - Invalid stock name (SQL injection attempt):");
        System.out.println(buyStock("AAPL'; DROP TABLE stock_purchases; --", 10));
        
        System.out.println("\\nTest Case 4 - Invalid quantity (negative):");
        System.out.println(buyStock("GOOGL", -5));
        
        System.out.println("\\nTest Case 5 - Invalid quantity (too large):");
        System.out.println(buyStock("MSFT", 2000000));
    }
}
