import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class Lse139 {

    /**
     * Connects to a PostgreSQL database using credentials from environment variables
     * and checks if a user is a moderator.
     *
     * Required Environment Variables:
     * DB_HOST: The database host (e.g., "localhost")
     * DB_PORT: The database port (e.g., "5432")
     * DB_NAME: The name of the database (e.g., "appdb")
     * DB_USER: The database username
     * DB_PASS: The database user's password
     *
     * Assumed table schema:
     * CREATE TABLE users (
     *     user_id SERIAL PRIMARY KEY,
     *     username VARCHAR(255) UNIQUE NOT NULL,
     *     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
     * );
     *
     * @param userId The ID of the user to check.
     * @return true if the user is a moderator, false otherwise or if an error occurs.
     */
    public static boolean isUserModerator(int userId) {
        String host = System.getenv("DB_HOST");
        String port = System.getenv("DB_PORT");
        String dbName = System.getenv("DB_NAME");
        String user = System.getenv("DB_USER");
        String password = System.getenv("DB_PASS");

        if (host == null || port == null || dbName == null || user == null || password == null) {
            System.err.println("Database environment variables are not set.");
            return false;
        }

        String url = "jdbc:postgresql://" + host + ":" + port + "/" + dbName;
        String sql = "SELECT is_moderator FROM users WHERE user_id = ?;";
        boolean isModerator = false;

        // Use try-with-resources to ensure connections are closed automatically
        try (Connection conn = DriverManager.getConnection(url, user, password);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            // Use PreparedStatement to prevent SQL injection
            pstmt.setInt(1, userId);

            try (ResultSet rs = pstmt.executeQuery()) {
                // If a row is found, get the boolean value
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
        System.out.println("--- Java Test Cases ---");
        System.out.println("Note: This requires a running PostgreSQL database and correctly set environment variables.");
        System.out.println("See comments in the isUserModerator function for details.");
        
        // Test cases (these will only work if you have a corresponding database setup)
        // Assume user 1 is a moderator, user 2 is not, and user 99 does not exist.
        int moderatorUserId = 1;
        int regularUserId = 2;
        int nonExistentUserId = 99;
        int anotherModeratorId = 3;
        int anotherRegularId = 4;

        System.out.printf("Is user %d a moderator? %b\n", moderatorUserId, isUserModerator(moderatorUserId));
        System.out.printf("Is user %d a moderator? %b\n", regularUserId, isUserModerator(regularUserId));
        System.out.printf("Is user %d a moderator? %b\n", nonExistentUserId, isUserModerator(nonExistentUserId));
        System.out.printf("Is user %d a moderator? %b\n", anotherModeratorId, isUserModerator(anotherModeratorId));
        System.out.printf("Is user %d a moderator? %b\n", anotherRegularId, isUserModerator(anotherRegularId));
    }
}