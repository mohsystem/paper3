// Chain-of-Through secure solution in Java (Single-file). Main class name: Task37
// Steps 1-5 embedded as comments for clarity and security.

// 1) Problem understanding:
//    - Connect to a SQL database (SQLite in-memory for demonstration).
//    - Execute a parameterized query to retrieve user info by username provided as input.
//    - Return a User object if found, otherwise null.

// 2) Security requirements:
//    - Use parameterized queries (PreparedStatement) to prevent SQL injection.
//    - Do not log sensitive data. Handle errors without exposing internals.
//    - Close resources using try-with-resources to avoid leaks.

// 3) Secure coding generation: Implemented below.
// 4) Code review: Ensured no string concatenation in SQL, all resources closed, safe error handling.
// 5) Secure code output: Final secured code below.

import java.sql.*;
import java.util.*;

public class Task37 {

    // Simple immutable User model for returning results
    public static final class User {
        public final String username;
        public final String fullName;
        public final String email;
        public final String role;

        public User(String username, String fullName, String email, String role) {
            this.username = username;
            this.fullName = fullName;
            this.email = email;
            this.role = role;
        }

        @Override
        public String toString() {
            return "User{username='" + username + "', fullName='" + fullName + "', email='" + email + "', role='" + role + "'}";
        }
    }

    // Establish a connection (SQLite in-memory for demonstration)
    public static Connection connect(String url) throws SQLException {
        // For JDBC 4+, the SQLite driver auto-registers if present on classpath.
        // No credentials are used for SQLite.
        return DriverManager.getConnection(url);
    }

    // Seed test data securely using prepared statements
    public static void seedTestData(Connection conn) throws SQLException {
        final String createSql = ""
                + "CREATE TABLE IF NOT EXISTS users ("
                + "  username TEXT PRIMARY KEY,"
                + "  full_name TEXT NOT NULL,"
                + "  email TEXT NOT NULL,"
                + "  role TEXT NOT NULL"
                + ");";
        try (Statement st = conn.createStatement()) {
            st.execute(createSql);
        }

        String insertSql = "INSERT INTO users (username, full_name, email, role) VALUES (?, ?, ?, ?)";
        try (PreparedStatement ps = conn.prepareStatement(insertSql)) {
            insertUser(ps, "alice", "Alice Johnson", "alice@example.com", "user");
            insertUser(ps, "bob", "Bob Smith", "bob@example.com", "user");
            insertUser(ps, "charlie", "Charlie Rose", "charlie@example.com", "manager");
            insertUser(ps, "admin", "System Admin", "admin@example.com", "admin");
            insertUser(ps, "eve", "Eve Adams", "eve@example.com", "auditor");
        }
    }

    private static void insertUser(PreparedStatement ps, String username, String fullName, String email, String role) throws SQLException {
        ps.setString(1, username);
        ps.setString(2, fullName);
        ps.setString(3, email);
        ps.setString(4, role);
        ps.executeUpdate();
    }

    // Securely retrieve user info by username using a prepared statement
    public static User getUserInfo(Connection conn, String username) throws SQLException {
        if (username == null) return null;
        final String sql = "SELECT username, full_name, email, role FROM users WHERE username = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, username);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return new User(
                            rs.getString("username"),
                            rs.getString("full_name"),
                            rs.getString("email"),
                            rs.getString("role")
                    );
                }
            }
        }
        return null;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String url = "jdbc:sqlite::memory:";
        try (Connection conn = connect(url)) {
            seedTestData(conn);

            // Test cases: includes normal lookups and an injection attempt
            String[] testUsernames = new String[] {
                "alice",
                "charlie",
                "nonexistent",
                "admin' OR '1'='1",
                "admin"
            };

            for (String uname : testUsernames) {
                try {
                    User u = getUserInfo(conn, uname);
                    System.out.println("Query username: " + uname + " -> " + (u == null ? "Not found" : u.toString()));
                } catch (SQLException e) {
                    // Minimal, safe error message (no sensitive details)
                    System.out.println("Error querying for username: " + uname);
                }
            }
        } catch (SQLException e) {
            System.out.println("Database error occurred.");
        }
    }
}