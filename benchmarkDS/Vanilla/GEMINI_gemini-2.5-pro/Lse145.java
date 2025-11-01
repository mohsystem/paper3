import java.util.HashMap;
import java.util.Map;

public class Lse145 {

    // Simulating a database table for stock holdings
    private static Map<String, Integer> portfolio = new HashMap<>();

    /**
     * This function is called by buy_stock to perform additional processing.
     * @param stockName The name of the stock.
     */
    public static void buy_function(String stockName) {
        System.out.println("-> buy_function: Performing additional checks/logging for " + stockName + "...");
    }

    /**
     * Simulates buying a stock and inserting it into a database.
     * This function takes the stock name and quantity and updates the portfolio.
     * It then calls the buy_function.
     * @param stockName The name of the stock to buy.
     * @param quantity The quantity of the stock to buy.
     */
    public static void buy_stock(String stockName, int quantity) {
        System.out.println("Attempting to buy " + quantity + " shares of " + stockName + ".");
        
        // Simulate inserting/updating the database
        portfolio.put(stockName, portfolio.getOrDefault(stockName, 0) + quantity);
        System.out.println("   Database updated: Portfolio now holds " + portfolio.get(stockName) + " shares of " + stockName + ".");
        
        // Call the other function as per requirement
        buy_function(stockName);
        System.out.println(); // for better readability of test cases
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");
        
        // Test Case 1
        buy_stock("AAPL", 10);
        
        // Test Case 2
        buy_stock("GOOGL", 5);
        
        // Test Case 3
        buy_stock("MSFT", 20);
        
        // Test Case 4: Buying more of an existing stock
        buy_stock("AAPL", 15);
        
        // Test Case 5
        buy_stock("AMZN", 2);
        
        System.out.println("--- Final Portfolio State ---");
        portfolio.forEach((stock, qty) -> System.out.println(stock + ": " + qty));
    }
}