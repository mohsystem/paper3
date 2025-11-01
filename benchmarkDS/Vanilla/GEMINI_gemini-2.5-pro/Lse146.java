import java.util.ArrayList;
import java.util.List;

class BuyOrder {
    private String username;
    private String stockName;
    private int quantity;

    public BuyOrder(String username, String stockName, int quantity) {
        this.username = username;
        this.stockName = stockName;
        this.quantity = quantity;
    }

    @Override
    public String toString() {
        return "Order[User: " + username + ", Stock: " + stockName + ", Quantity: " + quantity + "]";
    }
}

public class Lse146 {

    // Simulating a database table with a static list
    private static List<BuyOrder> ordersDatabase = new ArrayList<>();

    /**
     * Simulates inserting a buy order into the database.
     *
     * @param stock_name   The name of the stock to buy.
     * @param stock_quantity The quantity of the stock to buy.
     * @param username     The user placing the order.
     * @return A confirmation message.
     */
    public static String placeBuyOrder(String stock_name, int stock_quantity, String username) {
        // Create a new order object
        BuyOrder newOrder = new BuyOrder(username, stock_name, stock_quantity);
        
        // The buy order is inserted into the database.
        ordersDatabase.add(newOrder);
        System.out.println("DATABASE: Successfully inserted order for " + username);

        // The user is redirected to the profile page.
        System.out.println("SYSTEM: Redirecting user '" + username + "' to the profile page.");

        return "Order for " + stock_quantity + " of " + stock_name + " has been placed successfully.";
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1
        System.out.println("\n--- Test Case 1 ---");
        String result1 = placeBuyOrder("AAPL", 10, "john_doe");
        System.out.println("RESULT: " + result1);

        // Test Case 2
        System.out.println("\n--- Test Case 2 ---");
        String result2 = placeBuyOrder("GOOGL", 5, "jane_smith");
        System.out.println("RESULT: " + result2);

        // Test Case 3
        System.out.println("\n--- Test Case 3 ---");
        String result3 = placeBuyOrder("TSLA", 20, "elon_fan");
        System.out.println("RESULT: " + result3);

        // Test Case 4
        System.out.println("\n--- Test Case 4 ---");
        String result4 = placeBuyOrder("AMZN", 2, "john_doe");
        System.out.println("RESULT: " + result4);

        // Test Case 5
        System.out.println("\n--- Test Case 5 ---");
        String result5 = placeBuyOrder("MSFT", 15, "bill_gates_fan");
        System.out.println("RESULT: " + result5);
        
        System.out.println("\n--- Final Database State ---");
        for (BuyOrder order : ordersDatabase) {
            System.out.println(order);
        }
    }
}