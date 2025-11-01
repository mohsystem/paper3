
import java.sql.*;
import java.util.regex.*;

public class Lse142 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/newsletter";
    private static final String DB_USER = "dbuser";
    private static final String DB_PASSWORD = "dbpassword";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"
    );
    
    public static String unsubscribe(String email) {
        // Input validation
        if (email == null || email.trim().isEmpty()) {
            return "Error: Email address is required";
        }
        
        email = email.trim();
        
        // Validate email format
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Error: Invalid email format";
        }
        
        Connection conn = null;
        PreparedStatement checkStmt = null;
        PreparedStatement deleteStmt = null;
        ResultSet rs = null;
        
        try {
            // Secure database connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Check if email exists using parameterized query
            String checkQuery = "SELECT email FROM subscribers WHERE email = ?";
            checkStmt = conn.prepareStatement(checkQuery);
            checkStmt.setString(1, email);
            rs = checkStmt.executeQuery();
            
            if (rs.next()) {
                // Email exists, remove it using parameterized query
                String deleteQuery = "DELETE FROM subscribers WHERE email = ?";
                deleteStmt = conn.prepareStatement(deleteQuery);
                deleteStmt.setString(1, email);
                int rowsAffected = deleteStmt.executeUpdate();
                
                if (rowsAffected > 0) {
                    return "Success: Email address has been unsubscribed";
                } else {
                    return "Error: Failed to unsubscribe";
                }
            } else {
                return "Info: Email address is not in the database";
            }
            
        } catch (SQLException e) {
            return "Error: Database operation failed";
        } finally {
            // Close resources in reverse order
            try {
                if (rs != null) rs.close();
                if (checkStmt != null) checkStmt.close();
                if (deleteStmt != null) deleteStmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                // Log error silently
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + unsubscribe("user@example.com"));
        System.out.println("Test 2: " + unsubscribe("test@domain.org"));
        System.out.println("Test 3: " + unsubscribe("invalid-email"));
        System.out.println("Test 4: " + unsubscribe(""));
        System.out.println("Test 5: " + unsubscribe("nonexistent@email.com"));
    }
}
