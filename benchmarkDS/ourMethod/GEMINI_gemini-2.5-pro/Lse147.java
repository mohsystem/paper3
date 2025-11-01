import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.ResultSet;
import java.util.regex.Pattern;
import java.io.File;

public class Lse147 {

    private static final String DB_FILENAME = "orders_java.db";
    private static final String DB_URL = "jdbc:sqlite:" + DB_FILENAME;
    private static final int MAX_STOCK_NAME_LENGTH = 10;
    // Simple regex for stock symbols: 1-10 uppercase letters and dots.
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[A-Z.]{1," + MAX_STOCK_NAME_LENGTH + "}$");

    public static void setupDatabase() {
        // Use try-with-resources to ensure the connection and statement are closed
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS orders (" +
                         "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                         "stock_name TEXT NOT NULL," +
                         "quantity INTEGER NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
        }
    }

    /**
     * Simulates a web route that takes stock name and quantity,
     * validates them, and inserts into the database.
     *
     * @param stockName The name of the stock to buy.
     * @param quantity The quantity of the stock to buy.
     * @return A message indicating success or failure.
     */
    public String buyOrder(String stockName, int quantity) {
        // Rule #1: Ensure all input is validated and sanitized
        if (stockName == null || !STOCK_NAME_PATTERN.matcher(stockName).matches()) {
            return "Error: Invalid stock name format. Must be 1-" + MAX_STOCK_NAME_LENGTH + " uppercase letters/dots.";
        }
        if (quantity <= 0) {
            return "Error: Quantity must be a positive integer.";
        }

        String sql = "INSERT INTO orders(stock_name, quantity) VALUES(?,?)";
        
        // Use try-with-resources for automatic resource management
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            // Rule #2 & #5: Use parameterized queries to prevent SQL injection
            pstmt.setString(1, stockName);
            pstmt.setInt(2, quantity);
            pstmt.executeUpdate();
            
            // Simulating redirection
            return "Order for " + quantity + " of " + stockName + " placed successfully. Redirecting to stock_view...";

        } catch (SQLException e) {
            System.err.println("Database insertion failed: " + e.getMessage());
            // Rule #5: Return generic error message
            return "Error: Could not process order due to a database error.";
        }
    }

    public static void viewStocks() {
        System.out.println("\n--- Current Orders in Database ---");
        String sql = "SELECT id, stock_name, quantity FROM orders";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            
            boolean found = false;
            while (rs.next()) {
                found = true;
                System.out.println("ID: " + rs.getInt("id") + ", " +
                                   "Stock: " + rs.getString("stock_name") + ", " +
                                   "Quantity: " + rs.getInt("quantity"));
            }
            if (!found) {
                System.out.println("No orders found.");
            }

        } catch (SQLException e) {
            System.err.println("Could not retrieve orders: " + e.getMessage());
        }
        System.out.println("--------------------------------\n");
    }

    public static void main(String[] args) {
        // Clean up previous database file for a fresh run
        new File(DB_FILENAME).delete();

        setupDatabase();
        Lse147 app = new Lse147();

        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid input
        System.out.println("Test 1: (Valid)");
        System.out.println("Input: stockName='AAPL', quantity=100");
        System.out.println("Output: " + app.buyOrder("AAPL", 100));
        System.out.println();

        // Test Case 2: Invalid quantity
        System.out.println("Test 2: (Invalid Quantity)");
        System.out.println("Input: stockName='GOOG', quantity=-5");
        System.out.println("Output: " + app.buyOrder("GOOG", -5));
        System.out.println();

        // Test Case 3: Invalid stock name (too long)
        System.out.println("Test 3: (Invalid Name - Too Long)");
        System.out.println("Input: stockName='VERYLONGSTOCKNAME', quantity=10");
        System.out.println("Output: " + app.buyOrder("VERYLONGSTOCKNAME", 10));
        System.out.println();
        
        // Test Case 4: Invalid stock name (invalid characters)
        System.out.println("Test 4: (Invalid Name - Characters)");
        System.out.println("Input: stockName='AMZN!', quantity=20");
        System.out.println("Output: " + app.buyOrder("AMZN!", 20));
        System.out.println();

        // Test Case 5: Potentially malicious input (blocked by validation)
        System.out.println("Test 5: (Malicious Input)");
        System.out.println("Input: stockName='TSLA'; --', quantity=50");
        System.out.println("Output: " + app.buyOrder("TSLA'; --", 50));
        System.out.println();

        // View the final state of the database
        viewStocks();
    }
}