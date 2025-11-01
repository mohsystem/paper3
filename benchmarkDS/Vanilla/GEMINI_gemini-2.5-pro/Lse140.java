/*
To run this Java code, you need to have the H2 database JAR in your classpath.
You can download it from the H2 database website or include it as a dependency
if you are using a build tool like Maven or Gradle.

Maven Dependency:
<dependency>
    <groupId>com.h2database</groupId>
    <artifactId>h2</artifactId>
    <version>2.2.224</version>
</dependency>
*/
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class Lse140 {

    // Database connection URL for an in-memory H2 database
    private static final String DB_URL = "jdbc:h2:mem:testdb;DB_CLOSE_DELAY=-1";
    private static final String USER = "sa";
    private static final String PASS = "";

    /**
     * Checks if a username exists in the users table.
     *
     * @param conn The database connection object.
     * @param username The username to check.
     * @return true if the username exists, false otherwise.
     */
    public static boolean checkUsernameExists(Connection conn, String username) {
        // SQL query to count occurrences of the username. Using prepared statements to prevent SQL injection.
        String sql = "SELECT COUNT(*) FROM users WHERE username = ?";
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    // If the count is greater than 0, the user exists.
                    return rs.getInt(1) > 0;
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * Sets up the database by creating a table and inserting sample data.
     * @param conn The database connection object.
     */
    private static void setupDatabase(Connection conn) throws SQLException {
        try (Statement stmt = conn.createStatement()) {
            // Create a simple 'users' table
            stmt.execute("CREATE TABLE users (id INT AUTO_INCREMENT PRIMARY KEY, username VARCHAR(255) NOT NULL UNIQUE)");
            // Insert some sample data
            stmt.execute("INSERT INTO users (username) VALUES ('alice')");
            stmt.execute("INSERT INTO users (username) VALUES ('bob')");
            stmt.execute("INSERT INTO users (username) VALUES ('carol')");
        }
    }

    public static void main(String[] args) {
        try (Connection conn = DriverManager.getConnection(DB_URL, USER, PASS)) {
            // Set up the database schema and data for testing
            setupDatabase(conn);
            System.out.println("Database connection established and setup complete.");

            // --- Test Cases ---
            System.out.println("\n--- Running Test Cases ---");

            // Test Case 1: Username that exists
            String user1 = "alice";
            System.out.println("1. Checking for username '" + user1 + "': " + (checkUsernameExists(conn, user1) ? "Exists" : "Does not exist"));

            // Test Case 2: Username that does not exist
            String user2 = "david";
            System.out.println("2. Checking for username '" + user2 + "': " + (checkUsernameExists(conn, user2) ? "Exists" : "Does not exist"));

            // Test Case 3: Another username that exists
            String user3 = "bob";
            System.out.println("3. Checking for username '" + user3 + "': " + (checkUsernameExists(conn, user3) ? "Exists" : "Does not exist"));

            // Test Case 4: Case-sensitive check (should not exist if DB is case-sensitive)
            String user4 = "Alice";
            System.out.println("4. Checking for username '" + user4 + "': " + (checkUsernameExists(conn, user4) ? "Exists" : "Does not exist"));
            
            // Test Case 5: Empty string username
            String user5 = "";
            System.out.println("5. Checking for username '" + user5 + "': " + (checkUsernameExists(conn, user5) ? "Exists" : "Does not exist"));

        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}