
import java.sql.*;
import java.util.Properties;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.regex.Pattern;

public class Lse147 {
    // Input validation: Whitelist pattern for stock names (alphanumeric and common symbols)
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[A-Z0-9._-]{1,10}$");
    private static final int MAX_QUANTITY = 1000000;
    private static final int MIN_QUANTITY = 1;

    // Secure database connection using properties file (avoid hardcoded credentials - CWE-798)
    private static Connection getConnection() throws SQLException, IOException {
        Properties props = new Properties();
        // In production, load from secure vault or environment variables
        props.setProperty("db.url", System.getenv("DB_URL") != null ? 
            System.getenv("DB_URL") : "jdbc:sqlite:stocks.db");
        props.setProperty("db.user", System.getenv("DB_USER") != null ? 
            System.getenv("DB_USER") : "stockuser");
        props.setProperty("db.password", System.getenv("DB_PASSWORD") != null ? 
            System.getenv("DB_PASSWORD") : "");
        
        return DriverManager.getConnection(
            props.getProperty("db.url"),
            props.getProperty("db.user"),
            props.getProperty("db.password")
        );
    }

    // Input validation function to prevent injection attacks (CWE-89)
    private static boolean isValidStockName(String stockName) {
        if (stockName == null || stockName.isEmpty()) {
            return false;
        }
        return STOCK_NAME_PATTERN.matcher(stockName).matches();
    }

    private static boolean isValidQuantity(int quantity) {
        return quantity >= MIN_QUANTITY && quantity <= MAX_QUANTITY;
    }

    // Secure buy order function using parameterized queries (prevents SQL injection)
    public static boolean buyOrder(String stockName, int quantity) {
        // Input validation: Check all inputs before processing (Rules#3)
        if (!isValidStockName(stockName)) {
            System.err.println("Invalid stock name format");
            return false;
        }

        if (!isValidQuantity(quantity)) {
            System.err.println("Invalid quantity range");
            return false;
        }

        Connection conn = null;
        PreparedStatement pstmt = null;

        try {
            conn = getConnection();
            
            // Use parameterized query to prevent SQL injection (CWE-89)
            String sql = "INSERT INTO orders (stock_name, quantity, order_date) VALUES (?, ?, datetime('now'))";
            pstmt = conn.prepareStatement(sql);
            
            // Set parameters - JDBC handles escaping automatically
            pstmt.setString(1, stockName);
            pstmt.setInt(2, quantity);
            
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                // Log success without exposing sensitive data (Rules#1)
                System.out.println("Order placed successfully");
                return true;
            } else {
                System.err.println("Order insertion failed");
                return false;
            }
            
        } catch (SQLException e) {
            // Error handling: Don't expose internal details to user (Rules#1)\n            System.err.println("Database error occurred");\n            // In production, log detailed error to secure log only\n            e.printStackTrace();\n            return false;\n        } catch (IOException e) {\n            System.err.println("Configuration error");\n            e.printStackTrace();\n            return false;\n        } finally {\n            // Resource cleanup: Always close resources (try-with-resources alternative)\n            try {\n                if (pstmt != null) pstmt.close();\n                if (conn != null) conn.close();\n            } catch (SQLException e) {\n                e.printStackTrace();\n            }\n        }\n    }\n\n    // Initialize database table\n    private static void initializeDatabase() {\n        try (Connection conn = getConnection();\n             Statement stmt = conn.createStatement()) {\n            \n            String createTable = "CREATE TABLE IF NOT EXISTS orders (" +\n                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +\n                "stock_name TEXT NOT NULL, " +\n                "quantity INTEGER NOT NULL, " +\n                "order_date TEXT NOT NULL)";\n            stmt.execute(createTable);\n            \n        } catch (Exception e) {\n            System.err.println("Database initialization failed");\n            e.printStackTrace();\n        }\n    }\n\n    public static void main(String[] args) {\n        // Initialize database\n        initializeDatabase();\n        \n        // Test cases with validation\n        System.out.println("Test Case 1: Valid order");\n        System.out.println("Result: " + buyOrder("AAPL", 100));\n        \n        System.out.println("\\nTest Case 2: Valid order with different stock");\n        System.out.println("Result: " + buyOrder("GOOGL", 50));\n        \n        System.out.println("\\nTest Case 3: Invalid stock name (SQL injection attempt)");\n        System.out.println("Result: " + buyOrder("'; DROP TABLE orders; --", 10));\n        \n        System.out.println("\
Test Case 4: Invalid quantity (negative)");\n        System.out.println("Result: " + buyOrder("MSFT", -5));\n        \n        System.out.println("\
Test Case 5: Invalid quantity (exceeds maximum)");\n        System.out.println("Result: " + buyOrder("TSLA", 2000000));
    }
}
