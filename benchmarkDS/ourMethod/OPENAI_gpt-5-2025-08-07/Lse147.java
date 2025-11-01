import java.nio.charset.StandardCharsets;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Lse147 {

    // Simple in-memory "database" to store buy orders safely
    static final class Order {
        final String stockName;
        final int quantity;
        final LocalDateTime createdAt;

        Order(String stockName, int quantity) {
            this.stockName = stockName;
            this.quantity = quantity;
            this.createdAt = LocalDateTime.now();
        }
    }

    static final class InMemoryDatabase {
        private final List<Order> orders = new ArrayList<>();

        public synchronized boolean insertOrder(String stockName, int quantity) {
            orders.add(new Order(stockName, quantity));
            return true;
        }

        public synchronized List<Order> getAllOrders() {
            return Collections.unmodifiableList(new ArrayList<>(orders));
        }
    }

    // Validation helpers
    private static final int MAX_STOCK_NAME_LEN = 64;
    private static final int MAX_QUANTITY = 1_000_000;

    public static String stock_view(InMemoryDatabase db) {
        StringBuilder sb = new StringBuilder();
        sb.append("Stock Orders:\n");
        List<Order> all = db.getAllOrders();
        int i = 1;
        for (Order o : all) {
            sb.append(i++)
              .append(". ")
              .append(o.stockName)
              .append(" - ")
              .append(o.quantity)
              .append(" @ ")
              .append(o.createdAt)
              .append("\n");
        }
        return sb.toString();
    }

    public static String render_form() {
        // Simulated HTML form (no server in this demo); provided for completeness
        return "<!doctype html>\n"
             + "<html><head><meta charset=\"utf-8\"><title>Buy Order</title></head>\n"
             + "<body>\n"
             + "<h1>Buy Order</h1>\n"
             + "<form method=\"POST\" action=\"/buy_order\">\n"
             + "  <label>Stock Name: <input type=\"text\" name=\"stock\" maxlength=\"64\" required></label><br>\n"
             + "  <label>Quantity: <input type=\"number\" name=\"qty\" min=\"1\" max=\"1000000\" required></label><br>\n"
             + "  <button type=\"submit\">Submit</button>\n"
             + "</form>\n"
             + "</body></html>\n";
    }

    public static String buy_order(InMemoryDatabase db, String stockNameInput, int quantityInput) {
        // Validate and sanitize inputs
        String stockName = sanitizeStockName(stockNameInput);
        if (!isValidStockName(stockName)) {
            return "Error: invalid stock name.";
        }
        if (!isValidQuantity(quantityInput)) {
            return "Error: invalid quantity.";
        }

        // "Insert into database"
        boolean ok = db.insertOrder(stockName, quantityInput);
        if (!ok) {
            return "Error: failed to insert order.";
        }

        // Simulate redirect to /stock_view by returning its content
        StringBuilder response = new StringBuilder();
        response.append("OK: Order inserted. Redirecting to /stock_view ...\n");
        response.append(stock_view(db));
        return response.toString();
    }

    private static String sanitizeStockName(String input) {
        if (input == null) return "";
        // Normalize to UTF-8 bytes then back to String to ensure valid encoding
        byte[] bytes = input.getBytes(StandardCharsets.UTF_8);
        String s = new String(bytes, StandardCharsets.UTF_8).trim();
        if (s.length() > MAX_STOCK_NAME_LEN) {
            s = s.substring(0, MAX_STOCK_NAME_LEN);
        }
        return s;
    }

    private static boolean isValidStockName(String s) {
        if (s == null) return false;
        int len = s.length();
        if (len < 1 || len > MAX_STOCK_NAME_LEN) return false;
        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            // Allowed: letters, digits, space, underscore, hyphen, dot
            if (!(Character.isLetterOrDigit(c) || c == ' ' || c == '_' || c == '-' || c == '.')) {
                return false;
            }
        }
        return true;
    }

    private static boolean isValidQuantity(int q) {
        return q >= 1 && q <= MAX_QUANTITY;
    }

    public static void main(String[] args) {
        InMemoryDatabase db = new InMemoryDatabase();

        System.out.println("== Rendered Form (Demo) ==");
        System.out.println(render_form());

        // Test cases (5):
        // 1) Valid
        System.out.println("Test 1: " + buy_order(db, "AAPL", 10));

        // 2) Valid
        System.out.println("Test 2: " + buy_order(db, "GOOG", 25));

        // 3) Invalid quantity (0)
        System.out.println("Test 3: " + buy_order(db, "MSFT", 0));

        // 4) Invalid stock name (disallowed chars)
        System.out.println("Test 4: " + buy_order(db, "DROP TABLE;--", 5));

        // 5) Valid with spaces and dot
        System.out.println("Test 5: " + buy_order(db, "BRK.B Class A", 100));
    }
}