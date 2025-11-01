import java.sql.*;
import java.util.Arrays;
import java.util.List;

public class Lse140 {
    // Securely checks if a username exists in the SQLite database at dbUrl.
    // Returns true if exists, false otherwise.
    public static boolean usernameExists(String dbUrl, String username) {
        if (username == null || username.isEmpty() || dbUrl == null || dbUrl.isEmpty()) {
            return false;
        }
        String sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
        try (Connection conn = DriverManager.getConnection(dbUrl);
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                return rs.next();
            }
        } catch (SQLException e) {
            // For security, avoid exposing internal details; return false on error.
            return false;
        }
    }

    // Helper to initialize the database for testing
    private static void initDb(String dbUrl) throws SQLException {
        try (Connection conn = DriverManager.getConnection(dbUrl)) {
            conn.setAutoCommit(false);
            try (Statement st = conn.createStatement()) {
                st.execute("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL)");
            }
            try (PreparedStatement ins = conn.prepareStatement("INSERT OR IGNORE INTO users (username) VALUES (?)")) {
                for (String u : new String[]{"alice", "bob", "eve"}) {
                    ins.setString(1, u);
                    ins.addBatch();
                }
                ins.executeBatch();
            }
            conn.commit();
        }
    }

    public static void main(String[] args) {
        String dbUrl = "jdbc:sqlite:users_java.db";
        try {
            initDb(dbUrl);
        } catch (SQLException e) {
            System.out.println("DB init failed");
            return;
        }

        List<String> tests = Arrays.asList(
                "alice",
                "bob",
                "charlie",
                "",
                "Robert'); DROP TABLE users;--"
        );

        for (String user : tests) {
            boolean exists = usernameExists(dbUrl, user);
            System.out.println("usernameExists(\"" + user + "\") = " + exists);
        }
    }
}