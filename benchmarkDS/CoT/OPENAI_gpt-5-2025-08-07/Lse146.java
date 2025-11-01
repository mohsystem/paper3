import java.sql.*;
import java.time.OffsetDateTime;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.regex.Pattern;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;

public class Lse146 {

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{1,64}$");
    private static final Pattern STOCK_PATTERN = Pattern.compile("^[A-Za-z0-9 _.-]{1,128}$");
    private static final int MAX_QTY = 1_000_000;
    private static final String DB_URL = "jdbc:sqlite:orders_java.db";

    public static String placeBuyOrder(String username, String stock_name, int stock_quantity) throws Exception {
        if (username == null || stock_name == null) {
            throw new IllegalArgumentException("Inputs cannot be null");
        }
        username = username.trim();
        stock_name = stock_name.trim();
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            throw new IllegalArgumentException("Invalid username");
        }
        if (!STOCK_PATTERN.matcher(stock_name).matches()) {
            throw new IllegalArgumentException("Invalid stock name");
        }
        if (stock_quantity <= 0 || stock_quantity > MAX_QTY) {
            throw new IllegalArgumentException("Invalid stock quantity");
        }

        String createdAt = OffsetDateTime.now(ZoneOffset.UTC).format(DateTimeFormatter.ISO_OFFSET_DATE_TIME);

        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            conn.setAutoCommit(true);
            try (Statement st = conn.createStatement()) {
                st.execute("CREATE TABLE IF NOT EXISTS buy_orders (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "username TEXT NOT NULL," +
                        "stock_name TEXT NOT NULL," +
                        "quantity INTEGER NOT NULL," +
                        "created_at TEXT NOT NULL" +
                        ")");
            }
            String sql = "INSERT INTO buy_orders (username, stock_name, quantity, created_at) VALUES (?, ?, ?, ?)";
            try (PreparedStatement ps = conn.prepareStatement(sql)) {
                ps.setString(1, username);
                ps.setString(2, stock_name);
                ps.setInt(3, stock_quantity);
                ps.setString(4, createdAt);
                ps.executeUpdate();
            }
        }

        String encodedUser = URLEncoder.encode(username, StandardCharsets.UTF_8.toString());
        return "/profile/" + encodedUser;
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
                {"alice", "AAPL", "10"},
                {"bob.smith", "GOOGL", "25"},
                {"charlie-d", "MSFT", "1000"},
                {"diana", "Tesla Inc", "15"},
                {"eve_99", "AMZN", "5"}
        };
        for (String[] t : tests) {
            try {
                String result = placeBuyOrder(t[0], t[1], Integer.parseInt(t[2]));
                System.out.println("Redirect to: " + result);
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
    }
}