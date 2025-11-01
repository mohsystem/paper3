import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public class Lse147 {

    public static void initDB(String dbPath) throws SQLException {
        try {
            Class.forName("org.sqlite.JDBC");
        } catch (ClassNotFoundException ignore) {
            // Driver may auto-load via JDBC 4.0
        }
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite:" + dbPath);
             Statement st = conn.createStatement()) {
            st.executeUpdate(
                "CREATE TABLE IF NOT EXISTS orders (" +
                "  id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "  stock TEXT NOT NULL," +
                "  qty INTEGER NOT NULL," +
                "  created_at TEXT DEFAULT CURRENT_TIMESTAMP" +
                ")"
            );
        }
    }

    public static long buy_order(String dbPath, String stock, int qty) throws SQLException {
        if (stock == null || stock.trim().isEmpty()) throw new IllegalArgumentException("stock is required");
        if (qty <= 0) throw new IllegalArgumentException("qty must be positive");
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite:" + dbPath);
             PreparedStatement ps = conn.prepareStatement(
                "INSERT INTO orders(stock, qty) VALUES(?, ?)",
                Statement.RETURN_GENERATED_KEYS
             )) {
            ps.setString(1, stock);
            ps.setInt(2, qty);
            ps.executeUpdate();
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) return rs.getLong(1);
            }
        }
        return -1L;
    }

    public static String stock_view(String dbPath) throws SQLException {
        StringBuilder sb = new StringBuilder();
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite:" + dbPath);
             PreparedStatement ps = conn.prepareStatement(
                 "SELECT id, stock, qty, created_at FROM orders ORDER BY id"
             );
             ResultSet rs = ps.executeQuery()) {
            while (rs.next()) {
                sb.append(rs.getLong("id")).append(",")
                  .append(rs.getString("stock")).append(",")
                  .append(rs.getInt("qty")).append(",")
                  .append(rs.getString("created_at")).append("\n");
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        String db = "stocks_java.db";
        initDB(db);

        // Simulate 5 form submissions (buy_order route) and redirect to stock_view
        buy_order(db, "AAPL", 10);
        System.out.print(stock_view(db));

        buy_order(db, "GOOG", 5);
        System.out.print(stock_view(db));

        buy_order(db, "MSFT", 20);
        System.out.print(stock_view(db));

        buy_order(db, "TSLA", 3);
        System.out.print(stock_view(db));

        buy_order(db, "AMZN", 7);
        System.out.print(stock_view(db));
    }
}