import java.util.regex.Pattern;

public class Lse146 {

    // Constants for validation
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_STOCK_NAME_LENGTH = 10;
    // Simple regex: alphanumeric, 3-50 chars
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9]{3," + MAX_USERNAME_LENGTH + "}$");
    // Simple regex: uppercase letters, 1-10 chars
    private static final Pattern STOCK_NAME_PATTERN = Pattern.compile("^[A-Z]{1," + MAX_STOCK_NAME_LENGTH + "}$");

    /**
     * Simulates inserting a buy order into a database using a secure, parameterized query pattern.
     *
     * @param username The user's name.
     * @param stockName The name of the stock.
     * @param quantity The quantity of stock to buy.
     */
    private static void insertBuyOrderIntoDB(String username, String stockName, int quantity) {
        // This is a simulation of a secure database insertion.
        // In a real application, this would use a PreparedStatement.
        System.out.println("DATABASE: Preparing to execute INSERT INTO buy_orders (username, stock_name, quantity) VALUES (?, ?, ?)");
        System.out.println("DATABASE: Binding parameter 1 (username) -> " + username);
        System.out.println("DATABASE: Binding parameter 2 (stock_name) -> " + stockName);
        System.out.println("DATABASE: Binding parameter 3 (quantity) -> " + quantity);
        System.out.println("DATABASE: Order for " + quantity + " of " + stockName + " by " + username + " inserted successfully.");
    }

    /**
     * Simulates redirecting the user to their profile page.
     *
     * @param username The user's name to build the profile URL.
     */
    private static void redirectToProfile(String username) {
        System.out.println("REDIRECT: Now redirecting to /profile/" + username);
    }

    /**
     * Processes a stock buy order after validating the inputs.
     *
     * @param username The user placing the order.
     * @param stockName The stock ticker symbol.
     * @param stockQuantity The number of shares to buy.
     * @return true if the order was processed successfully, false otherwise.
     */
    public static boolean processBuyOrder(String username, String stockName, int stockQuantity) {
        // Rule #1: Input Validation
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Error: Invalid username format or length.");
            return false;
        }

        if (stockName == null || !STOCK_NAME_PATTERN.matcher(stockName).matches()) {
            System.err.println("Error: Invalid stock name format. Must be 1-" + MAX_STOCK_NAME_LENGTH + " uppercase letters.");
            return false;
        }

        if (stockQuantity <= 0) {
            System.err.println("Error: Stock quantity must be a positive number.");
            return false;
        }

        System.out.println("Input validation passed.");

        // Simulate database insertion
        insertBuyOrderIntoDB(username, stockName, stockQuantity);

        // Simulate redirection
        redirectToProfile(username);

        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Valid Order ---");
        processBuyOrder("testuser123", "GOOGL", 10);
        System.out.println("\n");

        System.out.println("--- Test Case 2: Invalid Username (special chars) ---");
        processBuyOrder("baduser!", "AAPL", 5);
        System.out.println("\n");

        System.out.println("--- Test Case 3: Invalid Stock Name (lowercase) ---");
        processBuyOrder("anotheruser", "msft", 20);
        System.out.println("\n");

        System.out.println("--- Test Case 4: Invalid Quantity (zero) ---");
        processBuyOrder("validuser", "TSLA", 0);
        System.out.println("\n");
        
        System.out.println("--- Test Case 5: Valid Order 2 ---");
        processBuyOrder("traderjoe", "AMZN", 2);
        System.out.println("\n");
    }
}