import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

/*
 * To compile and run this code, you need to have the SQLite JDBC driver.
 * 1. Download the driver JAR (e.g., from https://github.com/xerial/sqlite-jdbc/releases)
 * 2. Compile: javac Lse140.java
 * 3. Run: java -cp ".:sqlite-jdbc-3.36.0.3.jar" Lse140 
 *    (Replace the JAR name with the one you downloaded)
 */
public class Lse140 {

    private static final String DB_URL = "jdbc:sqlite:user_database.db";

    /**
     * Sets up the database by creating a users table and inserting sample data.
     * This method is for demonstration purposes.
     */
    public static void setupDatabase() {
        // The try-with-resources statement ensures that each resource is closed at the end of the statement
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            // Drop table if it exists to start fresh for the demo
            stmt.execute("DROP TABLE IF EXISTS users");
            
            // Create a simple users table
            String sql = "CREATE TABLE users (" +
                         "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                         "username TEXT NOT NULL UNIQUE)";
            stmt.execute(sql);

            // Insert sample data
            stmt.execute("INSERT INTO users (username) VALUES ('alice')");
            stmt.execute("INSERT INTO users (username) VALUES ('bob')");
            stmt.execute("INSERT INTO users (username) VALUES ('charlie')");
            
        } catch (SQLException e) {
            System.err.println("Database setup error: " + e.getMessage());
        }
    }

    /**
     * Securely checks if a username exists in the database.
     * Uses a PreparedStatement to prevent SQL injection attacks.
     *
     * @param username The username to check.
     * @return true if the username exists, false otherwise.
     */
    public static boolean usernameExists(String username) {
        // Using a PreparedStatement is crucial for security to prevent SQL injection.
        // The '?' is a placeholder for the parameter.
        String sql = "SELECT 1 FROM users WHERE username = ?";
        
        // Using try-with-resources ensures the connection and statement are closed automatically.
        // NOTE: In a real-world application, avoid hardcoding database credentials.
        // Use a configuration file, environment variables, or a secrets management service.
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            // Bind the username parameter to the placeholder in the SQL query.
            // This treats the input as a literal value, not as executable SQL code.
            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                // If rs.next() is true, it means the query returned at least one row,
                // so the username exists.
                return rs.next();
            }
        } catch (SQLException e) {
            System.err.println("Database query error: " + e.getMessage());
            // In case of an error, we assume the user doesn't exist or is inaccessible.
            return false;
        }
    }

    public static void main(String[] args) {
        // Setup a temporary database for the example
        setupDatabase();

        // --- 5 Test Cases ---
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Check for an existing user
        String user1 = "alice";
        System.out.println("Does '" + user1 + "' exist? " + usernameExists(user1)); // Expected: true

        // Test Case 2: Check for a non-existent user
        String user2 = "david";
        System.out.println("Does '" + user2 + "' exist? " + usernameExists(user2)); // Expected: false

        // Test Case 3: Check for another existing user (case-sensitive)
        String user3 = "Bob";
        System.out.println("Does '" + user3 + "' exist? " + usernameExists(user3)); // Expected: false (SQLite is case-sensitive by default)

        // Test Case 4: SQL Injection attempt. Should be handled safely.
        String user4 = "' OR '1'='1";
        System.out.println("Does '" + user4 + "' exist? " + usernameExists(user4)); // Expected: false

        // Test Case 5: Check for an empty string
        String user5 = "";
        System.out.println("Does '" + user5 + "' exist? " + usernameExists(user5)); // Expected: false
    }
}