import java.util.HashMap;
import java.util.Map;

public class Lse145 {

    // Simulating a database table for stock portfolio using a HashMap
    private static Map<String, Integer> portfolio = new HashMap<>();

    /**
     * This function is called by buy_stock to perform a subsequent action.
     * In a real application, this could be logging, sending a notification, etc.
     *
     * @param stockName The name of the stock that was purchased.
     */
    public static void buy_function(String stockName) {
        System.out.println("LOG: Post-purchase action for " + stockName + " completed.");
    }

    /**
     * Simulates buying a stock. It takes the stock name and quantity,
     * validates the input, "inserts" it into the simulated database,
     * and then calls a subsequent function.
     *
     * @param stockName The ticker symbol of the stock to buy (e.g., "AAPL").
     * @param quantity  The number of shares to buy.
     */
    public static void buy_stock(String stockName, int quantity) {
        System.out.println("\nAttempting to buy " + quantity + " of " + stockName + "...");

        // --- Security: Input Validation ---
        // 1. Ensure stockName is not null or empty.
        // 2. Ensure quantity is a positive number.
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("Error: Stock name cannot be empty.");
            return;
        }
        if (quantity <= 0) {
            System.out.println("Error: Quantity must be greater than zero.");
            return;
        }
        
        // Sanitize input to prevent issues, e.g., trim whitespace, convert to uppercase
        String sanitizedStockName = stockName.trim().toUpperCase();

        System.out.println("Inserting " + quantity + " of " + sanitizedStockName + " into the database.");
        
        // --- "Database" Insertion ---
        // In a real application, this would use a PreparedStatement to prevent SQL injection.
        // Example:
        // String query = "INSERT INTO portfolio (stock_name, quantity) VALUES (?, ?) ON DUPLICATE KEY UPDATE quantity = quantity + ?;";
        // PreparedStatement pstmt = connection.prepareStatement(query);
        // pstmt.setString(1, sanitizedStockName);
        // pstmt.setInt(2, quantity);
        // pstmt.setInt(3, quantity);
        // pstmt.executeUpdate();
        
        // Simulating the database update
        portfolio.put(sanitizedStockName, portfolio.getOrDefault(sanitizedStockName, 0) + quantity);
        System.out.println("Successfully updated portfolio for " + sanitizedStockName + ".");

        // Call the subsequent function as per the prompt
        buy_function(sanitizedStockName);
    }

    public static void main(String[] args) {
        System.out.println("--- Starting Stock Purchase Simulation ---");

        // Test Case 1: Buy a new stock
        buy_stock("GOOG", 10);

        // Test Case 2: Buy more of an existing stock
        buy_stock("GOOG", 5);

        // Test Case 3: Buy another new stock
        buy_stock("MSFT", 20);

        // Test Case 4: Invalid quantity
        buy_stock("TSLA", -5);

        // Test Case 5: Invalid stock name
        buy_stock("  ", 100);

        System.out.println("\n--- Final Portfolio Status ---");
        if (portfolio.isEmpty()) {
            System.out.println("Portfolio is empty.");
        } else {
            portfolio.forEach((stock, quantity) -> 
                System.out.println("Stock: " + stock + ", Quantity: " + quantity)
            );
        }
    }
}