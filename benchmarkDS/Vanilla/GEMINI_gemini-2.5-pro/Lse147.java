import java.util.HashMap;
import java.util.Map;

/**
 * Main class to simulate buying stocks.
 * This simulates the server-side logic of a web application.
 * In a real web application (e.g., using Spring Boot or Servlets), the buyOrder
 * method would be called by a controller handling a POST request from a form.
 */
public class Lse147 {

    // Using a HashMap to simulate a database table for stocks.
    // The key is the stock name (String), and the value is the quantity (Integer).
    private final Map<String, Integer> stockDatabase;

    public Lse147() {
        this.stockDatabase = new HashMap<>();
        System.out.println("Initialized a new stock portfolio (simulated database).");
    }

    /**
     * Simulates the buy_order route logic.
     * It takes the stock name and quantity, inserts/updates it in the database,
     * and simulates a redirect.
     *
     * @param stockName The name of the stock to buy.
     * @param quantity  The quantity of the stock to buy.
     */
    public void buyOrder(String stockName, int quantity) {
        System.out.println("\n--- Processing Buy Order ---");
        System.out.println("Received form data: stockName='" + stockName + "', quantity=" + quantity);

        // Insert/Update logic for the database
        // If stock already exists, add to its quantity. Otherwise, add the new stock.
        stockDatabase.put(stockName, stockDatabase.getOrDefault(stockName, 0) + quantity);

        System.out.println("DATABASE: Successfully inserted/updated order for " + stockName + " with quantity " + quantity + ".");
        System.out.println("ACTION: Redirecting user to the stock_view page...");
    }

    /**
     * Simulates the stock_view page by displaying all current holdings.
     */
    public void viewStocks() {
        System.out.println("\n--- Displaying Stock View Page ---");
        System.out.println("Current Portfolio Holdings:");
        if (stockDatabase.isEmpty()) {
            System.out.println("Portfolio is empty.");
        } else {
            for (Map.Entry<String, Integer> entry : stockDatabase.entrySet()) {
                System.out.println("- Stock: " + entry.getKey() + ", Quantity: " + entry.getValue());
            }
        }
        System.out.println("---------------------------------");
    }

    public static void main(String[] args) {
        Lse147 portfolioManager = new Lse147();

        // --- 5 Test Cases ---

        // Test Case 1: Buy a new stock
        portfolioManager.buyOrder("AAPL", 10);

        // Test Case 2: Buy another new stock
        portfolioManager.buyOrder("GOOGL", 5);

        // Test Case 3: Buy more of an existing stock
        portfolioManager.buyOrder("AAPL", 15);

        // Test Case 4: Buy a stock with a different name
        portfolioManager.buyOrder("TSLA", 8);

        // Test Case 5: Buy a small quantity of another stock
        portfolioManager.buyOrder("MSFT", 2);

        // Display the final state of the portfolio, simulating the stock_view page
        portfolioManager.viewStocks();
    }
}