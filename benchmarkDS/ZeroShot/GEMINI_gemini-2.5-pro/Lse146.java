import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.regex.Pattern;

// This is a simulation. No actual database connection is made.
// The code demonstrates the secure use of PreparedStatement to prevent SQL injection.
public class Lse146 {

    // Simulates a database connection object. In a real application, this would be
    // a connection from a connection pool (e.g., HikariCP).
    private static final Connection dbConnection = null; 
    
    // Basic validation pattern for stock names (alphanumeric, dot, dash)
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9.-]+$");
    // Basic validation pattern for usernames
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,20}$");


    /**
     * Securely inserts a buy order into the database using a PreparedStatement.
     * This prevents SQL injection attacks.
     *
     * @param username The user placing the order.
     * @param stockName The name of the stock.
     * @param stockQuantity The quantity of the stock to buy.
     * @return A string indicating the result of the operation.
     */
    public String buyStock(String username, String stockName, int stockQuantity) {
        // 1. Input Validation
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            return "Error: Invalid username format.";
        }
        if (stockName == null || stockName.trim().isEmpty() || !STOCK_NAME_PATTERN.matcher(stockName).matches()) {
            return "Error: Invalid stock name format.";
        }
        if (stockQuantity <= 0) {
            return "Error: Stock quantity must be a positive number.";
        }

        // 2. Use PreparedStatement for security
        String sql = "INSERT INTO buy_orders (username, stock_name, quantity, order_date) VALUES (?, ?, ?, NOW());";

        System.out.println("\n--- New Order ---");
        System.out.println("Executing secure buy order...");
        System.out.println("Username: " + username);
        System.out.println("Stock: " + stockName);
        System.out.println("Quantity: " + stockQuantity);
        
        // In a real application, you would uncomment the following block.
        // Here, we simulate the process.
        /*
        try (PreparedStatement preparedStatement = dbConnection.prepareStatement(sql)) {
            preparedStatement.setString(1, username);
            preparedStatement.setString(2, stockName);
            preparedStatement.setInt(3, stockQuantity);

            int rowsAffected = preparedStatement.executeUpdate();
            if (rowsAffected > 0) {
                 return "Success: Order placed for " + username + ". Redirecting to profile page.";
            } else {
                 return "Error: Failed to place order.";
            }
        } catch (SQLException e) {
            // Log the exception securely
            System.err.println("Database error: " + e.getMessage());
            return "Error: A database error occurred.";
        }
        */
        
        // Simulation of the prepared statement
        System.out.println("Simulating PreparedStatement...");
        System.out.println("SQL Template: " + sql);
        System.out.println("Binding parameter 1 (String): " + username);
        System.out.println("Binding parameter 2 (String): " + stockName);
        System.out.println("Binding parameter 3 (int): " + stockQuantity);
        System.out.println("Simulation successful.");

        return "Success: Order placed for " + username + ". Redirecting to profile page.";
    }

    public static void main(String[] args) {
        Lse146 portfolio = new Lse146();

        // Test Case 1: Normal valid transaction
        System.out.println(portfolio.buyStock("jdoe", "GOOGL", 10));

        // Test Case 2: Another valid transaction
        System.out.println(portfolio.buyStock("asmith", "TSLA", 5));

        // Test Case 3: SQL Injection attempt (should be handled safely as a literal string)
        System.out.println(portfolio.buyStock("evil_user", "FAKE.STK'); DROP TABLE buy_orders; --", 1));

        // Test Case 4: Invalid quantity
        System.out.println(portfolio.buyStock("jdoe", "MSFT", -50));
        
        // Test Case 5: Invalid stock name
        System.out.println(portfolio.buyStock("asmith", "INVALID$STOCK", 100));
    }
}