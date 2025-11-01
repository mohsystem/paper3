
import java.sql.*;

public class Lse140 {
    public static boolean checkUsernameExists(String username, String dbUrl, String dbUser, String dbPassword) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Connect to the database
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            
            // Execute query to check if username exists
            String query = "SELECT COUNT(*) FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                int count = rs.getInt(1);
                return count > 0;
            }
            
            return false;
        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void main(String[] args) {
        String dbUrl = "jdbc:mysql://localhost:3306/testdb";
        String dbUser = "root";
        String dbPassword = "password";
        
        // Test cases
        System.out.println("Test 1: " + checkUsernameExists("john_doe", dbUrl, dbUser, dbPassword));
        System.out.println("Test 2: " + checkUsernameExists("jane_smith", dbUrl, dbUser, dbPassword));
        System.out.println("Test 3: " + checkUsernameExists("admin", dbUrl, dbUser, dbPassword));
        System.out.println("Test 4: " + checkUsernameExists("test_user", dbUrl, dbUser, dbPassword));
        System.out.println("Test 5: " + checkUsernameExists("nonexistent", dbUrl, dbUser, dbPassword));
    }
}
