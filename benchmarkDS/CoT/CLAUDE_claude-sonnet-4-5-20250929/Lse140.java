
import java.sql.*;

public class Lse140 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/testdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static boolean checkUsernameExists(String username) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            String query = "SELECT COUNT(*) FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                return rs.getInt(1) > 0;
            }
            return false;
        } catch (SQLException e) {
            System.err.println("Database error occurred");
            return false;
        } finally {
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources");
            }
        }
    }
    
    public static void main(String[] args) {
        String[] testUsers = {"admin", "john_doe", "'; DROP TABLE users; --", 
                              "user123", "nonexistent"};
        
        System.out.println("Testing username existence:");
        for (String user : testUsers) {
            boolean exists = checkUsernameExists(user);
            System.out.println("Username '" + user + "' exists: " + exists);
        }
    }
}
