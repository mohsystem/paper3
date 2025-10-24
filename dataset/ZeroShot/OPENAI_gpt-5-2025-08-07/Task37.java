import java.sql.*;
import java.util.Arrays;

public class Task37 {

    // Securely retrieve user info with parameterized query. Returns JSON string or "NOT_FOUND".
    public static String getUserInfo(Connection conn, String username) throws SQLException {
        if (conn == null) throw new IllegalArgumentException("Connection cannot be null");
        if (username == null) return "NOT_FOUND";

        String sql = "SELECT username, full_name, email, created_at FROM users WHERE username = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setQueryTimeout(5); // seconds
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    String uname = rs.getString("username");
                    String fullName = rs.getString("full_name");
                    String email = rs.getString("email");
                    String createdAt = rs.getString("created_at");
                    return "{" +
                            "\"username\":" + jsonEscape(uname) + "," +
                            "\"full_name\":" + jsonEscape(fullName) + "," +
                            "\"email\":" + jsonEscape(email) + "," +
                            "\"created_at\":" + jsonEscape(createdAt) +
                            "}";
                } else {
                    return "NOT_FOUND";
                }
            }
        }
    }

    // Basic JSON string escaper that wraps the result in quotes and escapes special chars.
    private static String jsonEscape(String s) {
        if (s == null) return "null";
        StringBuilder sb = new StringBuilder();
        sb.append('"');
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '"': sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int)c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        sb.append('"');
        return sb.toString();
    }

    private static void exec(Connection conn, String sql) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.setQueryTimeout(5); // seconds
            st.execute(sql);
        }
    }

    public static void main(String[] args) {
        // Using in-memory SQLite DB for demo. Requires SQLite JDBC driver in classpath.
        // Driver class is typically org.sqlite.JDBC, but DriverManager can auto-load in modern JDKs with proper service files.
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:")) {
            try (Statement st = conn.createStatement()) {
                st.execute("PRAGMA foreign_keys = ON");
                st.execute("PRAGMA busy_timeout = 5000");
            }

            exec(conn, "CREATE TABLE users (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                    "username TEXT UNIQUE NOT NULL," +
                    "full_name TEXT NOT NULL," +
                    "email TEXT NOT NULL," +
                    "created_at TEXT NOT NULL" +
                    ")");

            // Seed sample data securely
            try (PreparedStatement ps = conn.prepareStatement(
                    "INSERT INTO users (username, full_name, email, created_at) VALUES (?, ?, ?, ?)")) {
                String[][] data = new String[][]{
                        {"alice", "Alice Anderson", "alice@example.com", "2023-01-01T00:00:00Z"},
                        {"bob", "Bob Brown", "bob@example.com", "2023-02:02T00:00:00Z".replace(':', '-')}, // keep valid string
                        {"carol", "Carol Clark", "carol@example.com", "2023-03-03T00:00:00Z"},
                        {"dave", "Dave Davis", "dave@example.com", "2023-04-04T00:00:00Z"}
                };
                for (String[] row : data) {
                    ps.setString(1, row[0]);
                    ps.setString(2, row[1]);
                    ps.setString(3, row[2]);
                    ps.setString(4, row[3]);
                    ps.addBatch();
                }
                ps.executeBatch();
            }

            // 5 test cases
            String[] testUsernames = new String[]{
                    "alice",
                    "bob",
                    "doesnotexist",
                    "carol",
                    "alice' OR '1'='1" // injection attempt that should safely return NOT_FOUND
            };
            for (String u : testUsernames) {
                try {
                    String result = getUserInfo(conn, u);
                    System.out.println("Query for '" + u + "': " + result);
                } catch (SQLException e) {
                    System.out.println("Error querying for '" + u + "': " + e.getMessage());
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
    }
}