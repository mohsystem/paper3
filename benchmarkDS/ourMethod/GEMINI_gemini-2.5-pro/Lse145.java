import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse145 {

    // Simulate a database table for stocks using a HashMap
    private static final Map<String, Integer> stockDatabase = new HashMap<>();
    private static final int MAX_STOCK_NAME_LENGTH = 16;
    // A regex to validate stock ticker format. Allows letters, numbers, dot, and hyphen.
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9.-]+$");

    /**
     * A secondary function called by buy_stock.
     * In a real application, this could perform logging, auditing, or trigger another process.
     * @param stockName The name of the stock being processed.
     */
    private static void buy_function(String stockName) {
        // Input is considered sanitized as it comes from the buy_stock function
        System.out.println("[buy_function] INFO: Secondary processing for stock: " + stockName);
    }

    /**
     * Simulates buying a stock by validating input and adding it to an in-memory database.
     * It calls a secondary function after a successful "purchase".
     *
     * @param stockName The name/ticker of the stock to buy.
     * @param quantity The amount of stock to buy.
     * @return true if the purchase was successful, false otherwise.
     */
    public static boolean buy_stock(String stockName, int quantity) {
        // Rule #1: Ensure all input is validated and sanitized
        // 1. Check for null or empty/whitespace-only string
        if (stockName == null || stockName.trim().isEmpty()) {
            System.err.println("ERROR: Stock name cannot be null or empty.");
            return false;
        }

        // 2. Trim the input to handle leading/trailing whitespace
        String sanitizedStockName = stockName.trim();

        // 3. Check length constraints
        if (sanitizedStockName.length() > MAX_STOCK_NAME_LENGTH) {
            System.err.println("ERROR: Stock name exceeds maximum length of " + MAX_STOCK_NAME_LENGTH + " characters.");
            return false;
        }

        // 4. Validate format/characters to prevent injection attacks
        if (!STOCK_NAME_PATTERN.matcher(sanitizedStockName).matches()) {
            System.err.println("ERROR: Stock name '" + sanitizedStockName + "' contains invalid characters.");
            return false;
        }
        
        // 5. Validate the quantity
        if (quantity <= 0) {
            System.err.println("ERROR: Quantity must be a positive number.");
            return false;
        }

        // --- Processing ---
        System.out.println("SUCCESS: Validation passed for " + quantity + " shares of " + sanitizedStockName + ".");
        
        // Simulate inserting/updating the record in the database
        // The compute method is atomic for this operation.
        stockDatabase.compute(sanitizedStockName, (key, oldQuantity) -> (oldQuantity == null) ? quantity : oldQuantity + quantity);
        System.out.println("DATABASE: Portfolio now holds " + stockDatabase.get(sanitizedStockName) + " shares of " + sanitizedStockName + ".");
        
        // Call the secondary function as required by the prompt
        buy_function(sanitizedStockName);
        
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Valid purchase
        System.out.println("\n--- Test Case 1: Valid Input ---");
        buy_stock("AAPL", 10);

        // Test Case 2: Invalid stock name (null)
        System.out.println("\n--- Test Case 2: Null Stock Name ---");
        buy_stock(null, 5);

        // Test Case 3: Invalid stock name (too long)
        System.out.println("\n--- Test Case 3: Stock Name Too Long ---");
        buy_stock("VERYLONGSTOCKNAMEISTOOLONG", 2);

        // Test Case 4: Invalid stock name (invalid characters like SQL injection attempt)
        System.out.println("\n--- Test Case 4: Invalid Characters ---");
        buy_stock("GOOG'; DROP TABLE stocks; --", 1);

        // Test Case 5: Invalid quantity (zero)
        System.out.println("\n--- Test Case 5: Invalid Quantity ---");
        buy_stock("MSFT", 0);
        
        System.out.println("\n--- Final Database State ---");
        System.out.println(stockDatabase);
    }
}