
import java.sql.*;
import java.util.Properties;

public class Task37 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static UserInfo getUserByUsername(String username) {
        // Input validation
        if (username == null || username.trim().isEmpty()) {
            System.out.println("Invalid username provided");
            return null;
        }
        
        // Limit username length to prevent buffer overflow
        if (username.length() > 50) {
            System.out.println("Username too long");
            return null;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        UserInfo userInfo = null;
        
        try {
            // Establish secure database connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Use parameterized query to prevent SQL injection
            String sql = "SELECT user_id, username, email, created_at FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            
            // Execute query
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                userInfo = new UserInfo(
                    rs.getInt("user_id"),
                    rs.getString("username"),
                    rs.getString("email"),
                    rs.getString("created_at")
                );
            }
        } catch (SQLException e) {
            // Log error without exposing sensitive information
            System.err.println("Database error occurred");
            e.printStackTrace();
        } finally {
            // Close resources properly
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing database resources");
            }
        }
        
        return userInfo;
    }
    
    static class UserInfo {
        private int userId;
        private String username;
        private String email;
        private String createdAt;
        
        public UserInfo(int userId, String username, String email, String createdAt) {
            this.userId = userId;
            this.username = username;
            this.email = email;
            this.createdAt = createdAt;
        }
        
        @Override
        public String toString() {
            return "UserInfo{userId=" + userId + ", username='" + username + 
                   "', email='" + email + "', createdAt='" + createdAt + "'}";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"john_doe", "alice", "bob123", "admin", "test_user"};
        
        System.out.println("Testing User Retrieval:\\n");
        for (String username : testUsers) {
            System.out.println("Searching for user: " + username);
            UserInfo user = getUserByUsername(username);
            if (user != null) {
                System.out.println("Found: " + user);
            } else {
                System.out.println("User not found or error occurred");
            }
            System.out.println("---");
        }
    }
}
