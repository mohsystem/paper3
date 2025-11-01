
import java.sql.*;
import java.util.regex.Pattern;
import java.nio.charset.StandardCharsets;
import java.util.Properties;
import java.io.InputStream;
import java.io.IOException;

public class Lse142 {
    // Rules#2: Input validation with strict email pattern
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$"
    );
    private static final int MAX_EMAIL_LENGTH = 254; // RFC 5321 maximum

    /**
     * Unsubscribe function with security controls
     * Rules#2: Validates input type, length, and format
     * Rules#32: Uses parameterized queries to prevent SQL injection
     * Rules#11: Returns generic error messages, logs details securely
     */
    public static String unsubscribe(String email) {
        // Rules#2: Validate input is not null
        if (email == null) {
            return "Invalid email address";
        }

        // Rules#2: Validate length to prevent buffer issues
        if (email.length() > MAX_EMAIL_LENGTH) {
            return "Invalid email address";
        }

        // Rules#2: Validate format using allow list pattern
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Invalid email address";
        }

        // Rules#3: Load database credentials from external configuration
        Properties props = new Properties();
        try (InputStream input = Lse142.class.getClassLoader()
                .getResourceAsStream("db.properties")) {
            if (input == null) {
                // Rules#11: Generic error to user, detailed to logs
                System.err.println("Unable to find db.properties");
                return "Service unavailable";
            }
            props.load(input);
        } catch (IOException e) {
            System.err.println("Error loading configuration: " + e.getMessage());
            return "Service unavailable";
        }

        String dbUrl = props.getProperty("db.url");
        String dbUser = props.getProperty("db.user");
        String dbPassword = props.getProperty("db.password");

        // Rules#32: Use parameterized queries
        String checkQuery = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
        String deleteQuery = "DELETE FROM subscribers WHERE email = ?";

        Connection conn = null;
        PreparedStatement checkStmt = null;
        PreparedStatement deleteStmt = null;
        ResultSet rs = null;

        try {
            // Rules#6: Ensure TLS connection if using network database
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            conn.setAutoCommit(false); // Use transaction for consistency

            // Check if email exists
            checkStmt = conn.prepareStatement(checkQuery);
            checkStmt.setString(1, email); // Rules#32: Parameterized to prevent SQL injection
            rs = checkStmt.executeQuery();

            if (rs.next() && rs.getInt(1) > 0) {
                // Email exists, remove it
                deleteStmt = conn.prepareStatement(deleteQuery);
                deleteStmt.setString(1, email); // Rules#32: Parameterized query
                int rowsAffected = deleteStmt.executeUpdate();
                
                if (rowsAffected > 0) {
                    conn.commit();
                    // Rules#10: Do not log email address (user data)
                    return "Successfully unsubscribed";
                } else {
                    conn.rollback();
                    return "Unsubscribe failed";
                }
            } else {
                // Email not in database
                conn.rollback();
                return "Email address not found in database";
            }

        } catch (SQLException e) {
            // Rules#11: Log detailed error, return generic message
            System.err.println("Database error: " + e.getMessage());
            if (conn != null) {
                try {
                    conn.rollback();
                } catch (SQLException rollbackEx) {
                    System.err.println("Rollback failed: " + rollbackEx.getMessage());
                }
            }
            return "Service error occurred";
        } finally {
            // Rules#37: Proper resource cleanup
            try {
                if (rs != null) rs.close();
                if (checkStmt != null) checkStmt.close();
                if (deleteStmt != null) deleteStmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Resource cleanup error: " + e.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - Valid email: " + unsubscribe("user@example.com"));
        System.out.println("Test 2 - Invalid format: " + unsubscribe("invalid-email"));
        System.out.println("Test 3 - Null input: " + unsubscribe(null));
        System.out.println("Test 4 - Too long: " + unsubscribe("a".repeat(300) + "@example.com"));
        System.out.println("Test 5 - SQL injection attempt: " + unsubscribe("user@example.com' OR '1'='1"));
    }
}
