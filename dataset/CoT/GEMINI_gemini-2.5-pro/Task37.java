import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

public class Task37 {

    // In-memory SQLite database URL
    private static final String DB_URL = "jdbc:sqlite::memory:";

    /**
     * Represents a user record.
     */
    static class User {
        int id;
        String username;
        String fullName;
        String email;

        User(int id, String username, String fullName, String email) {
            this.id = id;
            this.username = username;
            this.fullName = fullName;
            this.email = email;
        }

        @Override
        public String toString() {
            return "User{" +
                   "id=" + id +
                   ", username='" + username + '\'' +
                   ", fullName='" + fullName + '\'' +
                   ", email='" + email + '\'' +
                   '}';
        }
    }

    /**
     * Sets up the database with a users table and some sample data.
     *
     * @param conn The database connection.
     * @throws SQLException if a database access error occurs.
     */
    public static void setupDatabase(Connection conn) throws SQLException {
        String createTableSQL = "CREATE TABLE users (" +
                                "id INTEGER PRIMARY KEY, " +
                                "username TEXT NOT NULL UNIQUE, " +
                                "full_name TEXT NOT NULL, " +
                                "email TEXT NOT NULL UNIQUE);";

        try (Statement stmt = conn.createStatement()) {
            stmt.execute(createTableSQL);
            // Insert sample data
            stmt.execute("INSERT INTO users(username, full_name, email) VALUES('alice', 'Alice Smith', 'alice@example.com');");
            stmt.execute("INSERT INTO users(username, full_name, email) VALUES('bob', 'Bob Johnson', 'bob@example.com');");
            stmt.execute("INSERT INTO users(username, full_name, email) VALUES('carol', 'Carol White', 'carol@example.com');");
        }
    }

    /**
     * Retrieves user information from the database based on the username.
     * Uses a PreparedStatement to prevent SQL injection.
     *
     * @param conn The database connection.
     * @param username The username to search for.
     * @return A User object if found, otherwise null.
     */
    public static User getUserInfo(Connection conn, String username) {
        // The SQL query with a placeholder (?) for the username
        String query = "SELECT id, username, full_name, email FROM users WHERE username = ?";
        User user = null;

        // Using try-with-resources to ensure PreparedStatement and ResultSet are closed
        try (PreparedStatement pstmt = conn.prepareStatement(query)) {
            // Bind the username parameter to the placeholder
            // This is the crucial step to prevent SQL injection
            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                // Check if a user was found
                if (rs.next()) {
                    user = new User(
                        rs.getInt("id"),
                        rs.getString("username"),
                        rs.getString("full_name"),
                        rs.getString("email")
                    );
                }
            }
        } catch (SQLException e) {
            // Log the error for debugging but don't expose details to the user
            System.err.println("Database query failed: " + e.getMessage());
        }
        return user;
    }

    public static void main(String[] args) {
        // Using try-with-resources to ensure the connection is closed
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            if (conn != null) {
                System.out.println("Connected to the in-memory SQLite database.");
                setupDatabase(conn);

                // --- Test Cases ---
                List<String> testUsernames = new ArrayList<>();
                testUsernames.add("alice"); // 1. Valid user
                testUsernames.add("bob");   // 2. Another valid user
                testUsernames.add("charlie"); // 3. Non-existent user
                testUsernames.add("' OR '1'='1' --"); // 4. SQL injection attempt
                testUsernames.add(""); // 5. Empty string username

                for (String username : testUsernames) {
                    System.out.println("\n--- Searching for user: '" + username + "' ---");
                    User foundUser = getUserInfo(conn, username);
                    if (foundUser != null) {
                        System.out.println("User found: " + foundUser);
                    } else {
                        System.out.println("User not found.");
                    }
                }
            }
        } catch (SQLException e) {
            System.err.println("Database connection error: " + e.getMessage());
        }
    }
}