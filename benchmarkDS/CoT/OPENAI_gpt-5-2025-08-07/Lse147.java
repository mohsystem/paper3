// Chain-of-Through process in code generation for Java:
// 1. Problem understanding: Implement a simple flow where a form submits a stock name and quantity to a "buy_order" route,
//    which inserts into a database and then redirects to "stock_view". We'll simulate this with methods:
//    - initDb(dbUrl)
//    - buyOrder(dbUrl, stockName, quantity) -> returns current stock view after insertion
//    - stockView(dbUrl) -> returns a list of rows.
// 2. Security requirements: Validate inputs (non-empty, length, allowed characters), use parameterized queries to prevent SQL injection,
//    handle resources safely (try-with-resources), and limit quantity range.
// 3. Secure coding generation: Implement prepared statements, input validation, and robust error handling.
// 4. Code review: Ensure statements and connections are closed, no dynamic SQL with concatenation, bounds checked, and exceptions handled.
// 5. Secure code output: Final code below applies mitigations and demonstrates 5 test cases.

import java.sql.*;
import java.util.*;

public class Lse147 {

    // Initialize database with a secure schema
    public static void initDb(String dbUrl) {
        String createTable = ""
                + "CREATE TABLE IF NOT EXISTS buy_orders ("
                + "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                + "  stock_name TEXT NOT NULL,"
                + "  quantity INTEGER NOT NULL CHECK(quantity > 0),"
                + "  created_at TEXT DEFAULT (datetime('now'))"
                + ");";
        try (Connection conn = DriverManager.getConnection(dbUrl);
             Statement st = conn.createStatement()) {
            st.execute(createTable);
        } catch (SQLException e) {
            throw new RuntimeException("DB init failed: " + e.getMessage(), e);
        }
    }

    // Input validation for stock name
    private static boolean isValidStockName(String name) {
        if (name == null) return false;
        String trimmed = name.trim();
        if (trimmed.isEmpty() || trimmed.length() > 64) return false;
        // Allow alphanumeric plus _ . - 
        for (int i = 0; i < trimmed.length(); i++) {
            char c = trimmed.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '.' || c == '-')) {
                return false;
            }
        }
        return true;
    }

    // Return current stock view after insertion
    public static List<String> buyOrder(String dbUrl, String stockName, int quantity) {
        if (!isValidStockName(stockName)) {
            throw new IllegalArgumentException("Invalid stock name");
        }
        if (quantity <= 0 || quantity > 1_000_000) {
            throw new IllegalArgumentException("Invalid quantity");
        }

        String insert = "INSERT INTO buy_orders (stock_name, quantity) VALUES (?, ?)";
        try (Connection conn = DriverManager.getConnection(dbUrl);
             PreparedStatement ps = conn.prepareStatement(insert)) {
            ps.setString(1, stockName.trim());
            ps.setInt(2, quantity);
            ps.executeUpdate();
        } catch (SQLException e) {
            throw new RuntimeException("Insert failed: " + e.getMessage(), e);
        }
        return stockView(dbUrl);
    }

    // Fetch current stock view
    public static List<String> stockView(String dbUrl) {
        String query = "SELECT id, stock_name, quantity, created_at FROM buy_orders ORDER BY id ASC";
        List<String> rows = new ArrayList<>();
        try (Connection conn = DriverManager.getConnection(dbUrl);
             PreparedStatement ps = conn.prepareStatement(query);
             ResultSet rs = ps.executeQuery()) {
            while (rs.next()) {
                rows.add(
                    rs.getInt("id") + "|" +
                    rs.getString("stock_name") + "|" +
                    rs.getInt("quantity") + "|" +
                    rs.getString("created_at")
                );
            }
        } catch (SQLException e) {
            throw new RuntimeException("Query failed: " + e.getMessage(), e);
        }
        return rows;
    }

    // 5 test cases
    public static void main(String[] args) {
        String dbUrl = "jdbc:sqlite:java_demo.db";
        initDb(dbUrl);

        List<String> view1 = buyOrder(dbUrl, "AAPL", 10);
        System.out.println("After AAPL: " + view1);

        List<String> view2 = buyOrder(dbUrl, "GOOGL", 5);
        System.out.println("After GOOGL: " + view2);

        List<String> view3 = buyOrder(dbUrl, "MSFT", 15);
        System.out.println("After MSFT: " + view3);

        List<String> view4 = buyOrder(dbUrl, "TSLA", 1);
        System.out.println("After TSLA: " + view4);

        List<String> view5 = buyOrder(dbUrl, "NVDA", 20);
        System.out.println("After NVDA: " + view5);
    }
}