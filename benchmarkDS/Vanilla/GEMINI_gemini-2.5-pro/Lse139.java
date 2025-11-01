import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * To run this code, you need:
 * 1. A PostgreSQL database.
 * 2. A 'users' table in the database with schema:
 *    CREATE TABLE users (
 *        id SERIAL PRIMARY KEY,
 *        username VARCHAR(50) UNIQUE NOT NULL,
 *        is_moderator BOOLEAN NOT NULL DEFAULT FALSE
 *    );
 * 3. The PostgreSQL JDBC driver in your classpath.
 *    You can download it or add it as a Maven/Gradle dependency.
 *    (e.g., <dependency><groupId>org.postgresql</groupId><artifactId>postgresql</artifactId><version>42.6.0</version></dependency>)
 * 4. The following environment variables set:
 *    DB_HOST=your_database_host
 *    DB_PORT=your_database_port (e.g., 5432)
 *    DB_NAME=your_database_name
 *    DB_USER=your_username
 *    DB_PASSWORD=your_password
 */
public class Lse139 {

    public static boolean isUserModerator(String username) {
        boolean isModerator = false;
        String host = System.getenv("DB_HOST");
        String port = System.getenv("DB_PORT");
        String dbName = System.getenv("DB_NAME");
        String user = System.getenv("DB_USER");
        String password = System.getenv("DB_PASSWORD");

        if (host == null || port == null || dbName == null || user == null || password == null) {
            System.err.println("Database environment variables are not set.");
            return false;
        }

        String url = "jdbc:postgresql://" + host + ":" + port + "/" + dbName;
        String sql = "SELECT is_moderator FROM users WHERE username = ?";

        // Using try-with-resources to ensure resources are closed automatically
        try (Connection conn = DriverManager.getConnection(url, user, password);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    isModerator = rs.getBoolean("is_moderator");
                }
            }

        } catch (SQLException e) {
            System.err.println("SQL Error: " + e.getMessage());
            // In a real application, you would log this error.
        }
        
        return isModerator;
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        // Note: These tests require a database connection and pre-populated data.
        // For example, insert a user 'admin' who is a moderator and 'guest' who is not.
        // INSERT INTO users (username, is_moderator) VALUES ('admin', TRUE);
        // INSERT INTO users (username, is_moderator) VALUES ('guest', FALSE);
        // INSERT INTO users (username, is_moderator) VALUES ('moderator1', TRUE);

        System.out.println("Checking 5 test cases. This requires a live database connection and pre-configured environment variables.");

        // Test Case 1: A user who is a moderator
        String user1 = "admin";
        System.out.println("Is '" + user1 + "' a moderator? " + isUserModerator(user1));

        // Test Case 2: A user who is not a moderator
        String user2 = "guest";
        System.out.println("Is '" + user2 + "' a moderator? " + isUserModerator(user2));

        // Test Case 3: Another user who is a moderator
        String user3 = "moderator1";
        System.out.println("Is '" + user3 + "' a moderator? " + isUserModerator(user3));
        
        // Test Case 4: A user who does not exist in the database
        String user4 = "nonexistent_user";
        System.out.println("Is '" + user4 + "' a moderator? " + isUserModerator(user4));

        // Test Case 5: A user with a potentially malicious name (handled by PreparedStatement)
        String user5 = "'; DROP TABLE users; --";
        System.out.println("Is '" + user5 + "' a moderator? " + isUserModerator(user5));
    }
}