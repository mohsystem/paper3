import java.sql.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.regex.Pattern;

public class Task37 {

    public static final class User {
        public final int id;
        public final String username;
        public final String email;
        public final String fullName;

        public User(int id, String username, String email, String fullName) {
            this.id = id;
            this.username = username;
            this.email = email;
            this.fullName = fullName;
        }

        @Override
        public String toString() {
            return "User{id=" + id + ", username='" + username + "', email='" + email + "', fullName='" + fullName + "'}";
        }
    }

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{1,30}$");

    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > 30) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    public static List<User> getUserInfo(Connection conn, String username) throws SQLException, IllegalArgumentException {
        Objects.requireNonNull(conn, "Connection must not be null");
        if (!isValidUsername(username)) {
            throw new IllegalArgumentException("Invalid username. Only alphanumeric and underscore allowed, length 1-30.");
        }
        String sql = "SELECT id, username, email, full_name FROM users WHERE username = ?";
        List<User> results = new ArrayList<>();
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    results.add(new User(
                        rs.getInt("id"),
                        rs.getString("username"),
                        rs.getString("email"),
                        rs.getString("full_name")
                    ));
                }
            }
        }
        return results;
    }

    public static void setupDatabase(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.execute("CREATE TABLE IF NOT EXISTS users (" +
                       "id INTEGER PRIMARY KEY," +
                       "username TEXT NOT NULL UNIQUE," +
                       "email TEXT NOT NULL," +
                       "full_name TEXT NOT NULL)");
        }
        String insert = "INSERT INTO users (id, username, email, full_name) VALUES (?, ?, ?, ?)";
        try (PreparedStatement ps = conn.prepareStatement(insert)) {
            conn.setAutoCommit(false);
            Object[][] data = new Object[][] {
                {1, "alice", "alice@example.com", "Alice Allison"},
                {2, "bob", "bob@example.com", "Bob Robertson"},
                {3, "carol", "carol@example.com", "Carol Carver"},
                {4, "dave_123", "dave@example.com", "Dave Davidson"},
                {5, "eve", "eve@example.com", "Eve Everson"}
            };
            for (Object[] row : data) {
                ps.setInt(1, (Integer) row[0]);
                ps.setString(2, (String) row[1]);
                ps.setString(3, (String) row[2]);
                ps.setString(4, (String) row[3]);
                try {
                    ps.executeUpdate();
                } catch (SQLException ignored) {
                    // ignore duplicates on repeated runs
                }
            }
            conn.commit();
            conn.setAutoCommit(true);
        }
    }

    public static void main(String[] args) {
        // Using SQLite in-memory DB via JDBC URL.
        // Note: Requires an SQLite JDBC driver on the classpath (e.g., org.xerial:sqlite-jdbc).
        String url = "jdbc:sqlite::memory:";
        try (Connection conn = DriverManager.getConnection(url)) {
            setupDatabase(conn);

            String[] testUsernames = new String[] {
                "alice",
                "bob",
                "carol",
                "nonexistent",
                "alice' OR '1'='1" // should be rejected by validation
            };

            for (String uname : testUsernames) {
                try {
                    List<User> users = getUserInfo(conn, uname);
                    System.out.println("Query for username '" + uname + "' -> " + users);
                } catch (IllegalArgumentException ex) {
                    System.out.println("Query for username '" + uname + "' rejected: " + ex.getMessage());
                } catch (SQLException ex) {
                    System.out.println("Database error querying username '" + uname + "': " + ex.getMessage());
                }
            }

        } catch (SQLException ex) {
            System.out.println("Database connection failed or driver missing: " + ex.getMessage());
        }
    }
}