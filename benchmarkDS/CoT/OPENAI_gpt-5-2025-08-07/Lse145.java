import java.sql.*;
import java.util.Arrays;

public class Lse145 {
    // Step 1: Problem understanding and setup
    // Purpose: Provide secure functions to process a stock purchase request by validating input,
    // invoking buy_function(stockName) for normalization/validation, and inserting into DB.
    // Step 2-5: Security requirements, secure coding, review, and secure output applied throughout.

    private static Connection conn;

    private static synchronized Connection getConnection() throws SQLException {
        if (conn == null || conn.isClosed()) {
            conn = DriverManager.getConnection("jdbc:sqlite::memory:");
            try (Statement st = conn.createStatement()) {
                st.execute("PRAGMA foreign_keys = ON;");
                st.execute(
                    "CREATE TABLE IF NOT EXISTS purchases (" +
                    "  id INTEGER PRIMARY KEY AUTOINCREMENT," +
                    "  stock_name TEXT NOT NULL," +
                    "  quantity INTEGER NOT NULL," +
                    "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP" +
                    ");"
                );
            }
        }
        return conn;
    }

    // buy_function: validates and normalizes stock name, returns uppercase symbol
    public static String buy_function(String stockName) {
        if (stockName == null) {
            throw new IllegalArgumentException("Stock name is required.");
        }
        String trimmed = stockName.trim();
        if (trimmed.isEmpty()) {
            throw new IllegalArgumentException("Stock name cannot be empty.");
        }
        if (!trimmed.matches("^[A-Za-z]{1,10}$")) {
            throw new IllegalArgumentException("Stock name must be 1-10 alphabetic characters.");
        }
        return trimmed.toUpperCase();
    }

    // buy_stock: validates quantity, calls buy_function, inserts into DB securely, returns new row id
    public static long buy_stock(String stockName, int quantity) throws SQLException {
        if (quantity <= 0 || quantity > 1_000_000) {
            throw new IllegalArgumentException("Quantity must be between 1 and 1,000,000.");
        }
        String symbol = buy_function(stockName);
        Connection c = getConnection();
        try (PreparedStatement ps = c.prepareStatement(
                "INSERT INTO purchases (stock_name, quantity) VALUES (?, ?)",
                Statement.RETURN_GENERATED_KEYS)) {
            ps.setString(1, symbol);
            ps.setInt(2, quantity);
            int count = ps.executeUpdate();
            if (count != 1) {
                throw new SQLException("Insert failed.");
            }
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) {
                    return rs.getLong(1);
                } else {
                    throw new SQLException("No generated key returned.");
                }
            }
        }
    }

    // Simple helper to run a test case safely
    private static void runTest(String name, String stock, int qty) {
        try {
            long id = buy_stock(stock, qty);
            System.out.println(name + " OK -> rowId=" + id);
        } catch (Exception e) {
            System.out.println(name + " ERR -> " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest("T1", "AAPL", 10);        // valid
        runTest("T2", "goog", 5);         // valid, case normalized
        runTest("T3", "BAD!", 3);         // invalid name
        runTest("T4", "DROP", 0);         // invalid quantity
        runTest("T5", "msft", 100);       // valid
    }
}