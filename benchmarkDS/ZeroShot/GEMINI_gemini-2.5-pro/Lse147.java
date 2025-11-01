import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Lse147 {

    // In-memory list to act as a simple database
    private static final List<Order> database = new ArrayList<>();

    // A simple record to represent a stock order
    public record Order(String stockName, int quantity) {}

    /**
     * Simulates buying a stock by validating input and adding it to an in-memory database.
     * This function demonstrates secure coding practices like input validation.
     *
     * @param stockName The name of the stock (e.g., "AAPL"). Must be alphanumeric and not empty.
     * @param quantity The number of shares to buy. Must be a positive integer.
     * @return A string indicating the result of the operation.
     */
    public static String buy_order(String stockName, int quantity) {
        // 1. Security: Validate stock name - ensure it's not null, empty, or just whitespace.
        if (stockName == null || stockName.trim().isEmpty()) {
            return "Error: Stock name cannot be empty.";
        }
        
        // 2. Security: Sanitize stock name - ensure it contains only allowed characters (alphanumeric).
        // This helps prevent injection attacks in a real database scenario.
        if (!Pattern.matches("^[a-zA-Z0-9]+$", stockName)) {
            return "Error: Stock name must be alphanumeric.";
        }

        // 3. Security: Validate quantity - ensure it's a positive number.
        if (quantity <= 0) {
            return "Error: Quantity must be greater than zero.";
        }

        // 4. "Insert" into the database if all checks pass.
        Order newOrder = new Order(stockName, quantity);
        database.add(newOrder);

        // 5. Simulate redirection by returning a success message.
        return "SUCCESS: Order for " + quantity + " shares of " + stockName + " placed. Redirecting to stock_view.";
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Valid order
        System.out.println("1. " + buy_order("AAPL", 100));

        // Test Case 2: Invalid quantity (zero)
        System.out.println("2. " + buy_order("GOOG", 0));

        // Test Case 3: Invalid stock name (contains special characters)
        System.out.println("3. " + buy_order("MSFT-X", 50));

        // Test Case 4: Invalid stock name (empty)
        System.out.println("4. " + buy_order("  ", 200));
        
        // Test Case 5: Valid order after some failures
        System.out.println("5. " + buy_order("TSLA", 25));

        // --- Display Database Content ---
        System.out.println("\n--- Current Database State ---");
        if (database.isEmpty()) {
            System.out.println("Database is empty.");
        } else {
            for (int i = 0; i < database.size(); i++) {
                Order order = database.get(i);
                System.out.println("Entry " + (i + 1) + ": Stock=" + order.stockName() + ", Quantity=" + order.quantity());
            }
        }
    }
}