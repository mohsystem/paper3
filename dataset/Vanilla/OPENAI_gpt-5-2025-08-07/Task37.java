import java.sql.*;
import java.util.Arrays;

public class Task37 {

    // Initialize the SQLite database with a users table and sample data
    public static void initDatabase(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.execute("CREATE TABLE IF NOT EXISTS users (" +
                    "id INTEGER PRIMARY KEY, " +
                    "username TEXT UNIQUE NOT NULL, " +
                    "full_name TEXT NOT NULL, " +
                    "email TEXT NOT NULL" +
                    ")");
        }
        try (PreparedStatement ps = conn.prepareStatement(
                "INSERT OR IGNORE INTO users (id, username, full_name, email) VALUES (?, ?, ?, ?)")) {
            Object[][] data = new Object[][]{
                    {1, "alice", "Alice Wonderland", "alice@example.com"},
                    {2, "bob", "Bob Builder", "bob@builder.com"},
                    {3, "carol", "Carol Singer", "carol@songs.org"},
                    {4, "dave", "Dave Grohl", "dave@foofighters.com"},
            };
            for (Object[] row : data) {
                ps.setInt(1, (Integer) row[0]);
                ps.setString(2, (String) row[1]);
                ps.setString(3, (String) row[2]);
                ps.setString(4, (String) row[3]);
                ps.addBatch();
            }
            ps.executeBatch();
        }
    }

    // Retrieve user by username and return a JSON string or null if not found
    public static String getUserByUsername(Connection conn, String username) throws SQLException {
        String sql = "SELECT id, username, full_name, email FROM users WHERE username = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    int id = rs.getInt("id");
                    String un = rs.getString("username");
                    String full = rs.getString("full_name");
                    String email = rs.getString("email");
                    return "{\"id\":" + id +
                            ",\"username\":\"" + jsonEscape(un) +
                            "\",\"full_name\":\"" + jsonEscape(full) +
                            "\",\"email\":\"" + jsonEscape(email) + "\"}";
                }
            }
        }
        return null;
    }

    private static String jsonEscape(String s) {
        if (s == null) return "";
        StringBuilder b = new StringBuilder();
        for (char c : s.toCharArray()) {
            switch (c) {
                case '\\': b.append("\\\\"); break;
                case '"': b.append("\\\""); break;
                case '\b': b.append("\\b"); break;
                case '\f': b.append("\\f"); break;
                case '\n': b.append("\\n"); break;
                case '\r': b.append("\\r"); break;
                case '\t': b.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        b.append(String.format("\\u%04x", (int)c));
                    } else {
                        b.append(c);
                    }
            }
        }
        return b.toString();
    }

    public static void main(String[] args) throws Exception {
        try {
            Class.forName("org.sqlite.JDBC");
        } catch (ClassNotFoundException ignore) {
            // If the driver is not explicitly available, SQLite JDBC may still auto-load depending on environment.
        }
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:")) {
            initDatabase(conn);
            String[] tests = {"alice", "bob", "carol", "dave", "nonexistent"};
            for (String t : tests) {
                String result = getUserByUsername(conn, t);
                System.out.println("Query for '" + t + "': " + (result != null ? result : "null"));
            }
        }
    }
}