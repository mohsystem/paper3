
import java.sql.*;

public class Lse140 {
    // Method to check if username exists in database
    public static boolean checkUsernameExists(String dbUrl, String dbUser, String dbPassword, String username) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Connect to the database
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            
            // Use parameterized query to prevent SQL injection
            String query = "SELECT COUNT(*) FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            
            // Execute query
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                int count = rs.getInt(1);
                return count > 0;
            }
            
            return false;
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        } finally {
            // Close resources in reverse order
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        // Database connection parameters (adjust as needed)
        String dbUrl = "jdbc:mysql://localhost:3306/testdb";
        String dbUser = "root";
        String dbPassword = "password";
        
        // Test cases
        System.out.println("Test Case 1: " + checkUsernameExists(dbUrl, dbUser, dbPassword, "admin"));
        System.out.println("Test Case 2: " + checkUsernameExists(dbUrl, dbUser, dbPassword, "user123"));
        System.out.println("Test Case 3: " + checkUsernameExists(dbUrl, dbUser, dbPassword, "testuser"));
        System.out.println("Test Case 4: " + checkUsernameExists(dbUrl, dbUser, dbPassword, "john_doe"));
        System.out.println("Test Case 5: " + checkUsernameExists(dbUrl, dbUser, dbPassword, "nonexistent"));
    }
}
