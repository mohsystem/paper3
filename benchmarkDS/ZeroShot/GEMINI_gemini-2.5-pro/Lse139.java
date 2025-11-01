import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Objects;

/**
 * Note: This code requires a PostgreSQL JDBC driver in the classpath.
 * You can add it to your project's dependencies (e.g., via Maven/Gradle).
 *
 * Before running, set the following environment variables:
 * DB_HOST: The database host (e.g., localhost)
 * DB_PORT: The database port (e.g., 5432)
 * DB_NAME: The database name
 * DB_USER: The database user
 * DB_PASSWORD: The database user's password
 *
 * Assumed SQL Schema:
 * CREATE TABLE users (
 *     user_id SERIAL PRIMARY KEY,
 *     username VARCHAR(50) UNIQUE NOT NULL,
 *     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
 * );
 */
public class Lse139 {

    /**
     * Checks if a user is a moderator by querying the database.
     * Credentials are read from environment variables.
     * Uses a PreparedStatement to prevent SQL injection.
     *
     * @param username The username to check.
     * @return True if the user is a moderator, false otherwise.
     */
    public static boolean isUserModerator(String username) {
        // 1. Get credentials securely from environment variables
        String dbHost = System.getenv("DB_HOST");
        String dbPort = System.getenv("DB_PORT");
        String dbName = System.getenv("DB_NAME");
        String dbUser = System.getenv("DB_USER");
        String dbPassword = System.getenv("DB_PASSWORD");

        if (dbHost == null || dbPort == null || dbName == null || dbUser == null || dbPassword == null) {
            System.err.println("Error: Database environment variables are not set.");
            return false;
        }
        
        // Ensure username is not null or empty
        if (username == null || username.trim().isEmpty()) {
            return false;
        }

        String url = "jdbc:postgresql://" + dbHost + ":" + dbPort + "/" + dbName;
        boolean isModerator = false;

        // 2. Use a PreparedStatement to prevent SQL injection
        String sql = "SELECT is_moderator FROM users WHERE username = ?";

        // 3. Use try-with-resources for automatic resource management
        try (Connection conn = DriverManager.getConnection(url, dbUser, dbPassword);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                // If a row is found, get the boolean value
                if (rs.next()) {
                    isModerator = rs.getBoolean("is_moderator");
                }
            }
        } catch (SQLException e) {
            // 4. Handle exceptions properly
            System.err.println("Database error: " + e.getMessage());
            // In a real application, you would log this error.
        }

        return isModerator;
    }

    public static void main(String[] args) {
        System.out.println("Running test cases...");
        System.out.println("Please ensure your database is running and environment variables are set.");
        System.out.println("Assumed test data:");
        System.out.println(" - user 'admin' is a moderator");
        System.out.println(" - user 'bob' is NOT a moderator");
        System.out.println(" - user 'charlie' does not exist");
        System.out.println("------------------------------------");

        // Test cases
        String[] testUsers = {
            "admin",               // Should be true
            "bob",                 // Should be false
            "charlie",             // Non-existent user, should be false
            "'; DROP TABLE users; --", // Malicious input, should be false and safe
            null                   // Null input
        };

        for (String user : testUsers) {
            try {
                boolean result = isUserModerator(user);
                System.out.printf("Is user '%s' a moderator? -> %b\n", user, result);
            } catch (Exception e) {
                System.err.printf("An error occurred while testing user '%s': %s\n", user, e.getMessage());
            }
        }
    }
}