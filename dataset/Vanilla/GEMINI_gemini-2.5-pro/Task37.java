import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

// Note: To run this code, you need to have the H2 database JAR in your classpath.
// You can download it from: https://www.h2database.com
// Or add it as a Maven/Gradle dependency.
public class Task37 {

    private static final String DB_URL = "jdbc:h2:mem:testdb"; // In-memory database
    private static final String USER = "sa";
    private static final String PASS = "";

    // Method to set up the database and table
    public static void setupDatabase(Connection conn) throws SQLException {
        try (Statement stmt = conn.createStatement()) {
            String createTableSql = "CREATE TABLE users (" +
                                    "id INT AUTO_INCREMENT PRIMARY KEY, " +
                                    "username VARCHAR(50) NOT NULL UNIQUE, " +
                                    "email VARCHAR(100) NOT NULL, " +
                                    "full_name VARCHAR(100))";
            stmt.execute(createTableSql);

            String insertSql = "INSERT INTO users (username, email, full_name) VALUES " +
                               "('alice', 'alice@example.com', 'Alice Smith')," +
                               "('bob', 'bob@example.com', 'Bob Johnson')," +
                               "('charlie', 'charlie@example.com', 'Charlie Brown')," +
                               "('diana', 'diana@example.com', 'Diana Prince')";
            stmt.execute(insertSql);
        }
    }

    /**
     * Connects to the database and retrieves user information.
     * @param conn The database connection object.
     * @param username The username to search for.
     * @return A string containing user information or a "not found" message.
     */
    public static String getUserInfo(Connection conn, String username) {
        // Using a PreparedStatement to prevent SQL injection
        String query = "SELECT username, email, full_name FROM users WHERE username = ?";
        StringBuilder result = new StringBuilder();

        try (PreparedStatement pstmt = conn.prepareStatement(query)) {
            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    result.append("User Found:\n");
                    result.append("  Username: ").append(rs.getString("username")).append("\n");
                    result.append("  Email: ").append(rs.getString("email")).append("\n");
                    result.append("  Full Name: ").append(rs.getString("full_name"));
                } else {
                    result.append("User '").append(username).append("' not found.");
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return "Error executing query: " + e.getMessage();
        }
        return result.toString();
    }

    public static void main(String[] args) {
        try (Connection conn = DriverManager.getConnection(DB_URL, USER, PASS)) {
            System.out.println("Connected to in-memory H2 database.");
            setupDatabase(conn);

            // 5 Test Cases
            String[] testUsernames = {"alice", "bob", "eve", "charlie", "diana"};

            for (int i = 0; i < testUsernames.length; i++) {
                System.out.println("\n--- Test Case " + (i + 1) + " ---");
                System.out.println("Searching for user: " + testUsernames[i]);
                String userInfo = getUserInfo(conn, testUsernames[i]);
                System.out.println(userInfo);
            }

        } catch (SQLException e) {
            System.err.println("Database connection failed!");
            e.printStackTrace();
        }
    }
}