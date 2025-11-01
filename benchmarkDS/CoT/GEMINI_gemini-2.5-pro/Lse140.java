import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class Lse140 {

    private static final String DB_URL = "jdbc:sqlite:users_java.db";

    /**
     * Sets up the database by creating a users table and inserting sample data.
     * This is for demonstration purposes to make the example runnable.
     */
    public static void setupDatabase() {
        // SQL statement for creating a new table
        String createTableSql = "CREATE TABLE IF NOT EXISTS users (\n"
                + " id integer PRIMARY KEY AUTOINCREMENT,\n"
                + " username text NOT NULL UNIQUE\n"
                + ");";

        // SQL statements for inserting data
        String[] insertSqls = {
            "INSERT OR IGNORE INTO users (username) VALUES('alice');",
            "INSERT OR IGNORE INTO users (username) VALUES('bob');",
            "INSERT OR IGNORE INTO users (username) VALUES('O''Malley');" // Note: SQL single quote is escaped
        };

        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            // Create table
            stmt.execute(createTableSql);
            // Insert sample users
            for (String sql : insertSqls) {
                stmt.execute(sql);
            }
        } catch (SQLException e) {
            System.err.println("Database setup error: " + e.getMessage());
        }
    }

    /**
     * Checks if a username exists in the database.
     *
     * @param username The username to check.
     * @return true if the username exists, false otherwise.
     */
    public static boolean usernameExists(String username) {
        // Using `SELECT 1` is efficient as we only care about existence, not the data.
        // The `?` is a placeholder for the parameter to prevent SQL injection.
        String sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";

        // Using try-with-resources to ensure database resources are automatically closed.
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            // Bind the username parameter to the prepared statement.
            // This is the key step in preventing SQL injection.
            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                // rs.next() returns true if a row was found, false otherwise.
                return rs.next();
            }

        } catch (SQLException e) {
            // In a real application, you would log this error.
            System.err.println("SQL Error: " + e.getMessage());
            // Return false in case of any database error for security.
            return false;
        }
    }

    public static void main(String[] args) {
        // Create and populate the database for the example
        setupDatabase();

        // --- Test Cases ---
        String[] testUsernames = {"alice", "bob", "charlie", "O'Malley", ""};

        System.out.println("--- Java Test Cases ---");
        for (String user : testUsernames) {
            boolean exists = usernameExists(user);
            System.out.printf("Does username '%s' exist? %b\n", user, exists);
        }
    }
}