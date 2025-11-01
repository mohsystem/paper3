import java.sql.*;

public class Lse140 {

    // Connect to the database using a JDBC URL (e.g., "jdbc:sqlite::memory:" for in-memory SQLite)
    public static Connection connect(String jdbcUrl) throws SQLException {
        return DriverManager.getConnection(jdbcUrl);
    }

    // Check if a username exists in the "users" table
    public static boolean checkUsernameExists(Connection conn, String username) throws SQLException {
        String sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                return rs.next();
            }
        }
    }

    // Create schema and insert sample data for testing
    public static void setupDatabase(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.execute("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY)");
        }
        try (PreparedStatement ps = conn.prepareStatement("INSERT OR IGNORE INTO users (username) VALUES (?)")) {
            String[] sampleUsers = {"alice", "bob", "charlie"};
            for (String u : sampleUsers) {
                ps.setString(1, u);
                ps.executeUpdate();
            }
        }
    }

    public static void main(String[] args) {
        String url = "jdbc:sqlite::memory:"; // Requires SQLite JDBC driver
        try (Connection conn = connect(url)) {
            setupDatabase(conn);

            String[] tests = {"alice", "bob", "charlie", "dave", "eve"}; // 5 test cases
            for (String user : tests) {
                boolean exists = checkUsernameExists(conn, user);
                System.out.println(user + " -> " + exists);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}