import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.regex.Pattern;

public class Task37 {

    private static final String DB_URL = "jdbc:sqlite::memory:";
    // A simple regex to validate a username: 3-16 alphanumeric characters.
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9]{3,16}$");

    // Initializes the database with a users table and some sample data
    public static void initializeDatabase(Connection conn) throws SQLException {
        String createTableSql = "CREATE TABLE IF NOT EXISTS users ("
                            + "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            + "username TEXT NOT NULL UNIQUE,"
                            + "email TEXT NOT NULL,"
                            + "full_name TEXT NOT NULL);";

        String insertDataSql1 = "INSERT INTO users(username, email, full_name) VALUES('alice', 'alice@example.com', 'Alice Smith');";
        String insertDataSql2 = "INSERT INTO users(username, email, full_name) VALUES('bob', 'bob@example.com', 'Bob Johnson');";

        try (Statement stmt = conn.createStatement()) {
            stmt.execute(createTableSql);
            stmt.execute(insertDataSql1);
            stmt.execute(insertDataSql2);
        }
    }

    // Validates the username format to ensure it meets requirements before being used.
    public static boolean isValidUsername(String username) {
        if (username == null) {
            return false;
        }
        return USERNAME_PATTERN.matcher(username).matches();
    }

    // Retrieves user information for a given username using a PreparedStatement to prevent SQL injection.
    public static String getUserInfo(Connection conn, String username) {
        if (!isValidUsername(username)) {
            return "Invalid username format.";
        }

        String sql = "SELECT id, username, email, full_name FROM users WHERE username = ?";
        StringBuilder result = new StringBuilder();

        // Use try-with-resources for PreparedStatement and ResultSet to ensure they are closed.
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    result.append("User Found:\n");
                    result.append("  ID: ").append(rs.getInt("id")).append("\n");
                    result.append("  Username: ").append(rs.getString("username")).append("\n");
                    result.append("  Email: ").append(rs.getString("email")).append("\n");
                    result.append("  Full Name: ").append(rs.getString("full_name"));
                } else {
                    result.append("User not found: ").append(username);
                }
            }
        } catch (SQLException e) {
            // In a real application, log this error to a secure logging system.
            System.err.println("Database query failed: " + e.getMessage());
            return "Error executing query.";
        }
        return result.toString();
    }

    public static void main(String[] args) {
        // NOTE: To compile and run this file, you need the SQLite JDBC driver.
        // Download it and include it in your classpath.
        // Example:
        // javac Task37.java
        // java -cp .:sqlite-jdbc-3.41.2.2.jar Task37
        
        // Use try-with-resources for the Connection to ensure it's always closed.
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            if (conn != null) {
                initializeDatabase(conn);
                
                System.out.println("--- Test Cases ---");
                
                // Test Case 1: Valid, existing user
                System.out.println("Test 1: Searching for 'alice'");
                System.out.println(getUserInfo(conn, "alice"));
                System.out.println("--------------------");

                // Test Case 2: Another valid, existing user
                System.out.println("Test 2: Searching for 'bob'");
                System.out.println(getUserInfo(conn, "bob"));
                System.out.println("--------------------");

                // Test Case 3: Valid, but non-existent user
                System.out.println("Test 3: Searching for 'charlie'");
                System.out.println(getUserInfo(conn, "charlie"));
                System.out.println("--------------------");

                // Test Case 4: Invalid username format (too short)
                System.out.println("Test 4: Searching for 'ad'");
                System.out.println(getUserInfo(conn, "ad"));
                System.out.println("--------------------");

                // Test Case 5: Classic SQL injection attempt.
                // This is safely handled by the PreparedStatement.
                System.out.println("Test 5: Searching for 'admin' OR '1'='1'");
                System.out.println(getUserInfo(conn, "admin' OR '1'='1'"));
                System.out.println("--------------------");
            }
        } catch (SQLException e) {
            System.err.println("Database connection failed: " + e.getMessage());
        }
    }
}