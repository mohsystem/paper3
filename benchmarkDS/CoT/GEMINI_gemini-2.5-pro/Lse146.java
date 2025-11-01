public class Lse146 {

    /**
     * Simulates placing a stock buy order with secure practices.
     *
     * @param username The username of the person placing the order.
     * @param stockName The name of the stock to buy.
     * @param stockQuantity The quantity of the stock to buy.
     */
    public static void placeBuyOrder(String username, String stockName, int stockQuantity) {
        // --- Input Validation ---
        // 1. Ensure username and stockName are not null or empty to prevent invalid data.
        if (username == null || username.trim().isEmpty()) {
            System.out.println("ERROR: Username cannot be empty.");
            return;
        }
        if (stockName == null || stockName.trim().isEmpty()) {
            System.out.println("ERROR: Stock name cannot be empty.");
            return;
        }
        // 2. Ensure quantity is a positive number.
        if (stockQuantity <= 0) {
            System.out.println("ERROR: Stock quantity must be a positive number.");
            return;
        }

        System.out.println("Processing order for user: " + username);

        // --- Simulate Secure Database Insertion ---
        // Using a parameterized query/prepared statement is crucial to prevent SQL Injection.
        // We simulate this by showing the query template and the bound parameters separately.
        System.out.println("DATABASE: Preparing statement: INSERT INTO buy_orders (username, stock_name, quantity) VALUES (?, ?, ?);");
        System.out.println("DATABASE: Binding parameter 1 (username) -> " + username);
        System.out.println("DATABASE: Binding parameter 2 (stock_name) -> " + stockName);
        System.out.println("DATABASE: Binding parameter 3 (quantity) -> " + stockQuantity);
        System.out.println("DATABASE: Execution successful. Order for " + stockQuantity + " of " + stockName + " inserted.");

        // --- Simulate Redirection ---
        System.out.println("SYSTEM: Redirecting user '" + username + "' to profile page.");
        System.out.println("----------------------------------------");
    }

    public static void main(String[] args) {
        // Test Case 1: Valid order
        placeBuyOrder("j_doe", "AAPL", 10);

        // Test Case 2: Another valid order
        placeBuyOrder("a_smith", "GOOGL", 5);

        // Test Case 3: Invalid order with zero quantity
        placeBuyOrder("b_jones", "TSLA", 0);

        // Test Case 4: Invalid order with empty stock name
        placeBuyOrder("c_white", "", 100);

        // Test Case 5: Invalid order with null username
        placeBuyOrder(null, "MSFT", 50);
    }
}