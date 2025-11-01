import java.sql.*;

public class Lse140 {

    // Function to check if a username exists. Returns true if exists, otherwise false.
    public static boolean usernameExists(Connection conn, String username) throws SQLException {
        if (conn == null || username == null) return false;
        String sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                return rs.next();
            }
        }
    }

    public static void main(String[] args) {
        // Using H2 in-memory database for demonstration. Requires H2 JDBC driver in classpath.
        String url = "jdbc:h2:mem:test;DB_CLOSE_DELAY=-1";
        try (Connection conn = DriverManager.getConnection(url)) {
            try (Statement st = conn.createStatement()) {
                st.execute("CREATE TABLE users (username VARCHAR(255) PRIMARY KEY)");
                st.execute("INSERT INTO users (username) VALUES ('alice')");
                st.execute("INSERT INTO users (username) VALUES ('bob')");
                st.execute("INSERT INTO users (username) VALUES ('carol')");
            }

            String[] testUsernames = new String[] {
                "alice",   // true
                "bob",     // true
                "dave",    // false
                "",        // false
                "Carol"    // false (case-sensitive)
            };

            for (String u : testUsernames) {
                boolean exists = usernameExists(conn, u);
                System.out.println("usernameExists(\"" + u + "\") = " + exists);
            }

        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}