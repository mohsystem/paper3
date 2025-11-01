import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.regex.Pattern;

public class Lse140 {

    // Validate username format: 3-20 characters, alphanumeric and underscores only.
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,20}$");

    /**
     * Connects to a database and checks if a username exists.
     * This function uses environment variables for database credentials.
     * export DB_URL="jdbc:mysql://localhost:3306/testdb"
     * export DB_USER="youruser"
     * export DB_PASS="yourpassword"
     *
     * @param username The username to check.
     * @return true if the username exists, false otherwise.
     */
    public static boolean usernameExists(String username) {
        // Rule #1: Ensure all input is validated.
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Invalid username format.");
            return false;
        }

        // Rule #5 & #6: Use environment variables for credentials.
        String dbUrl = System.getenv("DB_URL");
        String dbUser = System.getenv("DB_USER");
        String dbPass = System.getenv("DB_PASS");

        if (dbUrl == null || dbUser == null || dbPass == null) {
            System.err.println("Database environment variables (DB_URL, DB_USER, DB_PASS) are not set.");
            return false;
        }

        boolean exists = false;
        // Using a prepared statement to prevent SQL injection (CWE-89).
        String query = "SELECT COUNT(*) FROM users WHERE username = ?";

        // Use try-with-resources for automatic resource management.
        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPass);
             PreparedStatement pstmt = conn.prepareStatement(query)) {

            pstmt.setString(1, username);

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    if (rs.getInt(1) > 0) {
                        exists = true;
                    }
                }
            }
        } catch (SQLException e) {
            // Rule #7: Return generic messages, log detailed ones securely.
            System.err.println("Database query failed. Error: " + e.getMessage());
            // In a real application, use a proper logger instead of System.err.
        }
        return exists;
    }

    // Helper method for main to set up the database for testing.
    private static void setupDatabaseForTesting() {
        String dbUrl = System.getenv("DB_URL");
        String dbUser = System.getenv("DB_USER");
        String dbPass = System.getenv("DB_PASS");

        if (dbUrl == null || dbUser == null || dbPass == null) {
            return; // Exit if not configured
        }
        
        String createTableSQL = "CREATE TABLE IF NOT EXISTS users ("
                            + "id INT AUTO_INCREMENT PRIMARY KEY,"
                            + "username VARCHAR(255) NOT NULL UNIQUE"
                            + ");";
        String insertSQL = "INSERT IGNORE INTO users (username) VALUES ('alice'), ('bob');";
        
        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPass);
             Statement stmt = conn.createStatement()) {
            stmt.execute(createTableSQL);
            stmt.executeUpdate(insertSQL);
            System.out.println("Database setup for testing complete.");
        } catch (SQLException e) {
            System.err.println("Could not set up database for testing: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Username Existence Check ---");
        System.out.println("This test requires a MySQL database and the following environment variables:");
        System.out.println("export DB_URL=\"jdbc:mysql://localhost:3306/testdb\"");
        System.out.println("export DB_USER=\"youruser\"");
        System.out.println("export DB_PASS=\"yourpassword\"");
        System.out.println("The user table will be created and populated if it doesn't exist.");
        System.out.println("-------------------------------------------------");
        
        // Setup a dummy table and users for testing purposes.
        setupDatabaseForTesting();

        // Test cases
        String[] testUsernames = {
            "alice",        // Test case 1: Existing user
            "charlie",      // Test case 2: Non-existent user
            "dave_has_a_very_long_name_that_is_invalid", // Test case 3: Invalid (too long)
            "eve-vil",      // Test case 4: Invalid (contains '-')
            ""              // Test case 5: Invalid (empty)
        };

        for (int i = 0; i < testUsernames.length; i++) {
            String username = testUsernames[i];
            System.out.printf("Test Case %d: Checking for username '%s'...\n", i + 1, username);
            boolean exists = usernameExists(username);
            System.out.printf("Result: Username '%s' %s.\n\n", username, exists ? "exists" : "does not exist or is invalid");
        }
    }
}