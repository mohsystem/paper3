
import java.sql.*;
import java.util.regex.Pattern;

public class Lse144 {
    private static final String EMAIL_REGEX = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(EMAIL_REGEX);
    
    // Database connection parameters - should be configured securely
    private static final String DB_URL = "jdbc:mysql://localhost:3306/emaildb";
    private static final String DB_USER = "dbuser";
    private static final String DB_PASSWORD = "dbpassword";
    
    /**
     * Validates email format
     */
    private static boolean isValidEmail(String email) {
        if (email == null || email.trim().isEmpty()) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches();
    }
    
    /**
     * Checks if email exists in database and unsubscribes if found
     * @param email The email address to unsubscribe
     * @return 1 if successfully unsubscribed, 0 otherwise
     */
    public static int unsubscribeEmail(String email) {
        // Validate email format first
        if (!isValidEmail(email)) {
            return 0;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Establish database connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Check if email exists using parameterized query (prevents SQL injection)
            String checkQuery = "SELECT email FROM subscribers WHERE email = ?";
            pstmt = conn.prepareStatement(checkQuery);
            pstmt.setString(1, email);
            rs = pstmt.executeQuery();
            
            // If email doesn't exist, return 0
            if (!rs.next()) {
                return 0;
            }
            
            // Email exists, call unsubscribe script
            int unsubscribeResult = callUnsubscribeScript(email);
            
            // If script returns 0 (success), return 1, otherwise return 0
            if (unsubscribeResult == 0) {
                return 1;
            } else {
                return 0;
            }
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return 0;
        } finally {
            // Clean up resources
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
    }
    
    /**
     * Simulates calling an unsubscribe script
     * @param email The email to unsubscribe
     * @return 0 on success, non-zero on failure
     */
    private static int callUnsubscribeScript(String email) {
        try {
            // In a real implementation, this would call an actual script
            // For security, validate and sanitize input before passing to external script
            
            // Simulate script execution
            // Return 0 for success
            return 0;
            
        } catch (Exception e) {
            System.err.println("Script execution error: " + e.getMessage());
            return 1;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 (valid existing email): " + unsubscribeEmail("user1@example.com"));
        System.out.println("Test Case 2 (valid non-existing email): " + unsubscribeEmail("nonexistent@example.com"));
        System.out.println("Test Case 3 (invalid email format): " + unsubscribeEmail("invalid-email"));
        System.out.println("Test Case 4 (null email): " + unsubscribeEmail(null));
        System.out.println("Test Case 5 (empty email): " + unsubscribeEmail(""));
    }
}
