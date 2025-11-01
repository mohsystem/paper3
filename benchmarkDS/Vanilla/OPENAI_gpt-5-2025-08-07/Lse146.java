import java.util.ArrayList;
import java.util.List;

public class Lse146 {

    static class Order {
        String username;
        String stock_name;
        int stock_quantity;
        long createdAt;
    }

    private static final List<Order> DATABASE = new ArrayList<>();

    public static String processBuyOrder(String stock_name, int stock_quantity, String username) {
        Order order = new Order();
        order.username = username;
        order.stock_name = stock_name;
        order.stock_quantity = stock_quantity;
        order.createdAt = System.currentTimeMillis();
        DATABASE.add(order);
        return "/users/" + username + "/profile";
    }

    public static void main(String[] args) {
        System.out.println(processBuyOrder("AAPL", 10, "alice"));
        System.out.println(processBuyOrder("GOOGL", 5, "bob"));
        System.out.println(processBuyOrder("TSLA", 2, "alice"));
        System.out.println(processBuyOrder("MSFT", 100, "carol"));
        System.out.println(processBuyOrder("AMZN", 7, "dave"));
    }
}