
import java.sql.*;
import java.util.regex.Pattern;

public class Lse145 {
    private static final String DB_URL = "jdbc:sqlite:stocks.db";
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[A-Z]{1,5}$");
    
    public static boolean buyStock(String stockName, int quantity) {
        // Input validation
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("Error: Stock name cannot be empty");
            return false;
        }
        
        if (!STOCK_NAME_PATTERN.matcher(stockName.trim().toUpperCase()).matches()) {
            System.out.println("Error: Invalid stock name format");
            return false;
        }
        
        if (quantity <= 0 || quantity > 10000) {
            System.out.println("Error: Quantity must be between 1 and 10000");
            return false;
        }
        
        // Parameterized query to prevent SQL injection
        String sql = "INSERT INTO stocks (stock_name, quantity) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, stockName.trim().toUpperCase());
            pstmt.setInt(2, quantity);
            pstmt.executeUpdate();
            
            // Call buy_function
            buyFunction(stockName.trim().toUpperCase());
            
            System.out.println("Stock purchased successfully");
            return true;
            
        } catch (SQLException e) {
            System.out.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    public static void buyFunction(String stockName) {
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("Error: Invalid stock name in buy_function");
            return;
        }
        System.out.println("Processing purchase for stock: " + stockName);
        // Additional business logic here
    }
    
    private static void initDatabase() {
        String createTable = "CREATE TABLE IF NOT EXISTS stocks (" +
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                           "stock_name TEXT NOT NULL, " +
                           "quantity INTEGER NOT NULL)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            stmt.execute(createTable);
        } catch (SQLException e) {
            System.out.println("Error creating table: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initDatabase();
        
        // Test cases
        System.out.println("Test 1: Valid purchase");
        buyStock("AAPL", 100);
        
        System.out.println("\\nTest 2: Valid purchase");
        buyStock("GOOGL", 50);
        
        System.out.println("\\nTest 3: Invalid stock name (too long)");
        buyStock("TOOLONG", 10);
        
        System.out.println("\\nTest 4: Invalid quantity (negative)");
        buyStock("MSFT", -5);
        
        System.out.println("\\nTest 5: Invalid stock name (empty)");
        buyStock("", 20);
    }
}
