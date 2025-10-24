// Note: To compile and run this code, you need to have the SQLite JDBC driver in your classpath.
// You can download it from: https://github.com/xerial/sqlite-jdbc/releases
// Example commands:
// javac Task37.java
// java -cp ".:sqlite-jdbc-3.41.2.2.jar" Task37  (use the version you downloaded)
import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public class Task37 {

    private static final String DB_URL = "jdbc:sqlite:user_database.db";

    /**
     * Sets up the database by creating a table and inserting sample data.
     * This method will drop the existing table if it exists.
     */
    public static void setupDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {

            stmt.execute("DROP TABLE IF EXISTS users");

            String createTableSQL = "CREATE TABLE users (" +
                                    "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                                    "username TEXT NOT NULL UNIQUE," +
                                    "full_name TEXT NOT NULL," +
                                    "email TEXT NOT NULL)";
            stmt.execute(createTableSQL);

            stmt.execute("INSERT INTO users (username, full_name, email) VALUES ('alice', 'Alice Smith', 'alice@example.com')");
            stmt.execute("INSERT INTO users (username, full_name, email) VALUES ('bob', 'Bob Johnson', 'bob@example.com')");
            stmt.execute("INSERT INTO users (username, full_name, email) VALUES ('admin', 'Administrator', 'admin@example.org')");
            System.out.println("Database setup successful.");
        } catch (SQLException e) {
            System.err.println("Database setup error: " + e.getMessage());
        }
    }

    /**
     * Retrieves user information from the database based on a username.
     * Uses a PreparedStatement to prevent SQL injection.
     *
     * @param username The username to search for.
     * @return A list of strings representing the user's data, or an empty list if not found.
     */
    public static List<String> getUserInfo(String username) {
        List<String> userInfo = new ArrayList<>();
        String sql = "SELECT id, username, full_name, email FROM users WHERE username = ?";

        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    userInfo.add("ID: " + rs.getInt("id"));
                    userInfo.add("Username: " + rs.getString("username"));
                    userInfo.add("Full Name: " + rs.getString("full_name"));
                    userInfo.add("Email: " + rs.getString("email"));
                }
            }
        } catch (SQLException e) {
            System.err.println("Query error: " + e.getMessage());
        }
        return userInfo;
    }

    public static void main(String[] args) {
        setupDatabase();

        System.out.println("\n--- Running 5 Test Cases ---");

        String[] testUsernames = {
            "alice",
            "bob",
            "charlie",
            "admin",
            "' OR '1'='1"
        };

        for (int i = 0; i < testUsernames.length; i++) {
            String username = testUsernames[i];
            System.out.println("\n[Test Case " + (i + 1) + "]");
            System.out.println("Searching for user: '" + username + "'");
            List<String> result = getUserInfo(username);
            if (result.isEmpty()) {
                System.out.println("Result: User not found.");
            } else {
                System.out.println("Result: User found:");
                result.forEach(line -> System.out.println("  " + line));
            }
        }
    }
}