import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;

public class Lse147 {

    private static boolean isValidStockName(String s) {
        if (s == null) return false;
        String name = s.trim();
        if (name.isEmpty() || name.length() > 50) return false;
        for (char ch : name.toCharArray()) {
            if (!(Character.isLetterOrDigit(ch) || ch == ' ' || ch == '.' || ch == '_' || ch == '-')) {
                return false;
            }
        }
        return true;
    }

    public static boolean initDb(String dbUrl) {
        String ddl = "CREATE TABLE IF NOT EXISTS buy_orders ("
                + "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                + "stock_name TEXT NOT NULL, "
                + "quantity INTEGER NOT NULL, "
                + "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
                + ");";
        try (Connection conn = DriverManager.getConnection(dbUrl);
             Statement st = conn.createStatement()) {
            st.execute(ddl);
            return true;
        } catch (SQLException e) {
            return false;
        }
    }

    // Simulates the buy_order route: validates input, inserts into DB securely, returns redirect path or error message.
    public static String buyOrder(String dbUrl, String stockName, int quantity) {
        if (!isValidStockName(stockName)) {
            return "ERROR: Invalid stock name";
        }
        if (quantity <= 0 || quantity > 1_000_000) {
            return "ERROR: Invalid quantity";
        }
        String trimmed = stockName.trim();

        try (Connection conn = DriverManager.getConnection(dbUrl)) {
            conn.setAutoCommit(false);
            String sql = "INSERT INTO buy_orders (stock_name, quantity) VALUES (?, ?)";
            try (PreparedStatement ps = conn.prepareStatement(sql)) {
                ps.setString(1, trimmed);
                ps.setInt(2, quantity);
                ps.executeUpdate();
                conn.commit();
                return "/stock_view";
            } catch (SQLException e) {
                try { conn.rollback(); } catch (SQLException ignored) {}
                return "ERROR: Database failure";
            }
        } catch (SQLException e) {
            return "ERROR: Database failure";
        }
    }

    public static void main(String[] args) {
        String dbUrl = "jdbc:sqlite:java_buy_orders.db";
        System.out.println("Init DB: " + initDb(dbUrl));

        // 5 test cases simulating form submissions to buy_order route
        System.out.println(buyOrder(dbUrl, "AAPL", 10));                         // expected redirect
        System.out.println(buyOrder(dbUrl, "MSFT-2025", 1));                      // expected redirect
        System.out.println(buyOrder(dbUrl, "AAPL; DROP TABLE buy_orders;", 5));   // expected error (invalid name)
        System.out.println(buyOrder(dbUrl, "GOOG", 0));                           // expected error (invalid qty)
        System.out.println(buyOrder(dbUrl, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 2)); // 51 'A's -> error
    }
}