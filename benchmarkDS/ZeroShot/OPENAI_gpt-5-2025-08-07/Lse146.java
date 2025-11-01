import java.sql.*;
import java.util.regex.Pattern;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;

public class Lse146 {

    private static final String DB_URL = "jdbc:sqlite:lse146_orders_java.db";
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,64}$");
    private static final Pattern STOCK_PATTERN = Pattern.compile("^[A-Za-z0-9 ._-]{1,64}$");

    private static void ensureTableExists(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.execute("CREATE TABLE IF NOT EXISTS buy_orders (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                    "username TEXT NOT NULL," +
                    "stock_name TEXT NOT NULL," +
                    "quantity INTEGER NOT NULL CHECK(quantity > 0)," +
                    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP" +
                    ")");
        }
    }

    private static void validateInputs(String username, String stockName, int quantity) {
        if (username == null || stockName == null) {
            throw new IllegalArgumentException("Inputs cannot be null");
        }
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            throw new IllegalArgumentException("Invalid username format");
        }
        if (!STOCK_PATTERN.matcher(stockName).matches()) {
            throw new IllegalArgumentException("Invalid stock name format");
        }
        if (quantity < 1 || quantity > 1_000_000) {
            throw new IllegalArgumentException("Quantity must be between 1 and 1,000,000");
        }
    }

    private static String urlEncode(String s) {
        return URLEncoder.encode(s, StandardCharsets.UTF_8);
    }

    // Returns redirect URL on success, or null on failure
    public static String placeBuyOrder(String username, String stock_name, int stock_quantity) {
        validateInputs(username, stock_name, stock_quantity);

        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            conn.setAutoCommit(false);
            ensureTableExists(conn);

            try (PreparedStatement ps = conn.prepareStatement(
                    "INSERT INTO buy_orders(username, stock_name, quantity) VALUES(?, ?, ?)")) {
                ps.setString(1, username);
                ps.setString(2, stock_name);
                ps.setInt(3, stock_quantity);
                ps.executeUpdate();
            }

            conn.commit();
            return "/profile/" + urlEncode(username);
        } catch (SQLException e) {
            // Do not leak DB details
            return null;
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] users = {"alice", "bob-01", "charlie", "dave", "eve"};
        String[] stocks = {"AAPL", "Microsoft Corp", "GOOG", "Bad'Stock", "TSLA"};
        int[] qtys = {10, 250, 0, 5, 100};

        // Expected: Success
        System.out.println("Test1: " + placeBuyOrder(users[0], stocks[0], qtys[0]));
        // Expected: Success (space in stock name)
        System.out.println("Test2: " + placeBuyOrder(users[1], stocks[1], qtys[1]));
        // Expected: Invalid quantity -> exception
        try {
            System.out.println("Test3: " + placeBuyOrder(users[2], stocks[2], qtys[2]));
        } catch (IllegalArgumentException ex) {
            System.out.println("Test3: INVALID - " + ex.getMessage());
        }
        // Expected: Invalid stock name (apostrophe)
        try {
            System.out.println("Test4: " + placeBuyOrder(users[3], stocks[3], qtys[3]));
        } catch (IllegalArgumentException ex) {
            System.out.println("Test4: INVALID - " + ex.getMessage());
        }
        // Expected: Success
        System.out.println("Test5: " + placeBuyOrder(users[4], stocks[4], qtys[4]));
    }
}