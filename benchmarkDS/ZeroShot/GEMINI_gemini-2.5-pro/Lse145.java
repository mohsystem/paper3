public class Lse145 {

    /**
     * This function simulates some secondary logic that runs after a stock purchase is recorded.
     * @param stockName The name of the stock.
     */
    public static void buy_function(String stockName) {
        System.out.println("-> buy_function: Executing secondary logic for stock: " + stockName);
    }

    /**
     * Simulates buying a stock and securely inserting the transaction into a database.
     * This function demonstrates input validation and the use of parameterized queries
     * to prevent SQL injection vulnerabilities.
     * @param stockName The name of the stock to buy (e.g., "AAPL").
     * @param quantity The number of shares to buy.
     */
    public static void buy_stock(String stockName, int quantity) {
        // --- Security: Input Validation ---
        // 1. Ensure stockName is not null and not just whitespace.
        // 2. Ensure quantity is a positive number.
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("Error: Stock name cannot be null or empty.\n");
            return;
        }
        if (quantity <= 0) {
            System.out.println("Error: Quantity must be a positive number.\n");
            return;
        }

        System.out.println("Processing purchase of " + quantity + " shares of " + stockName);

        // --- Security: Using Parameterized Queries (Prepared Statements) ---
        // This is the standard way to prevent SQL injection. The database driver
        // ensures that the input values are treated as data, not as executable code.
        
        // An insecure concatenation would be vulnerable:
        // String query = "INSERT INTO stocks (name, quantity) VALUES ('" + stockName + "', " + quantity + ")";
        
        // The secure, parameterized approach (simulated):
        System.out.println("Simulating secure database insertion...");
        System.out.println("  Prepared Statement: INSERT INTO stocks (name, quantity) VALUES (?, ?)");
        System.out.println("  Binding parameter 1 (String): " + stockName);
        System.out.println("  Binding parameter 2 (int): " + quantity);
        System.out.println("  Execution successful.");

        // Call the other function as required.
        buy_function(stockName);
        System.out.println(); // Add a newline for cleaner test output.
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---\n");

        // Test Case 1: Valid input
        buy_stock("AAPL", 10);

        // Test Case 2: Another valid input
        buy_stock("GOOG", 5);

        // Test Case 3: Invalid quantity (zero)
        buy_stock("MSFT", 0);

        // Test Case 4: Invalid stock name (null)
        buy_stock(null, 100);

        // Test Case 5: Invalid stock name (whitespace)
        buy_stock("  ", 20);
    }
}