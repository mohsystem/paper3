import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Objects;

// To compile and run this code, you need the PostgreSQL JDBC driver.
// 1. Download the driver JAR from https://jdbc.postgresql.org/download/
// 2. Compile: javac Lse139.java
// 3. Run with driver in classpath (replace with your actual path): 
//    java -cp ".:/path/to/postgresql-driver.jar" Lse139
//
// Before running, set the following environment variables:
// export DB_HOST=localhost
// export DB_PORT=5432
// export DB_NAME=yourdbname
// export DB_USER=youruser
// export DB_PASS=yourpassword
//
// And create the required table and data in your PostgreSQL database:
// CREATE TABLE users (
//     id SERIAL PRIMARY KEY,
//     username VARCHAR(50) UNIQUE NOT NULL,
//     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
// );
// INSERT INTO users (username, is_moderator) VALUES ('alice', TRUE);
// INSERT INTO users (username, is_moderator) VALUES ('bob', FALSE);

public class Lse139 {

    /**
     * Checks if a user is a moderator by querying a database using credentials
     * from environment variables. It uses a PreparedStatement to prevent SQL injection.
     *
     * @param username The username to check. Must not be null or empty.
     * @return true if the user exists and is a moderator, false otherwise.
     */
    public static boolean isUserModerator(String username) {
        // Rule #1: Ensure all input is validated.
        if (username == null || username.trim().isEmpty()) {
            System.err.println("Error: Username cannot be null or empty.");
            return false;
        }
        
        // Rules #6, #7: Use environment variables for credentials.
        String dbHost = System.getenv("DB_HOST");
        String dbPort = System.getenv("DB_PORT");
        String dbName = System.getenv("DB_NAME");
        String dbUser = System.getenv("DB_USER");
        String dbPass = System.getenv("DB_PASS");

        if (Objects.isNull(dbHost) || Objects.isNull(dbPort) || Objects.isNull(dbName) ||
            Objects.isNull(dbUser) || Objects.isNull(dbPass)) {
            System.err.println("Error: Database environment variables not set.");
            System.err.println("Please set DB_HOST, DB_PORT, DB_NAME, DB_USER, and DB_PASS.");
            return false;
        }

        String url = String.format("jdbc:postgresql://%s:%s/%s", dbHost, dbPort, dbName);
        String sql = "SELECT is_moderator FROM users WHERE username = ?";
        boolean isModerator = false;

        // Use try-with-resources to ensure database resources are automatically closed.
        try (Connection conn = DriverManager.getConnection(url, dbUser, dbPass);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            // Use PreparedStatement to prevent SQL injection.
            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                // Check if a row was returned.
                if (rs.next()) {
                    isModerator = rs.getBoolean("is_moderator");
                }
            }
        } catch (SQLException e) {
            // In a production environment, log this error to a secure logging system.
            System.err.println("Database query failed: " + e.getMessage());
            return false; // Fail closed on any exception.
        }
        return isModerator;
    }

    public static void main(String[] args) {
        System.out.println("Running moderator check tests...");
        System.out.println("---------------------------------");
        System.out.println("This test assumes a PostgreSQL database is running and");
        System.out.println("the required environment variables and table are set up.");
        System.out.println("---------------------------------");

        // 5 Test cases
        String[] testUsers = {
            "alice",          // Expected: true (existing moderator)
            "bob",            // Expected: false (existing non-moderator)
            "eve",            // Expected: false (non-existent user)
            "' OR '1'='1",    // Expected: false (SQL injection attempt)
            ""                // Expected: false (invalid input)
        };

        for (String user : testUsers) {
            System.out.printf("Checking if user '%s' is a moderator... ", user.isEmpty() ? "<empty>" : user);
            boolean result = isUserModerator(user);
            System.out.println(result);
        }
    }
}