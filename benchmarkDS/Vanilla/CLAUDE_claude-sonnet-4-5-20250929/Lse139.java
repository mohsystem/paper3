
import java.sql.*;

public class Lse139 {
    public static boolean isModerator(String username) {
        String dbUrl = System.getenv("DB_URL");
        String dbUser = System.getenv("DB_USER");
        String dbPassword = System.getenv("DB_PASSWORD");
        
        if (dbUrl == null || dbUser == null || dbPassword == null) {
            System.err.println("Database credentials not found in environment variables");
            return false;
        }
        
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        
        try {
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            String query = "SELECT is_moderator FROM users WHERE username = ?";
            stmt = conn.prepareStatement(query);
            stmt.setString(1, username);
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                return rs.getBoolean("is_moderator");
            }
            return false;
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        } finally {
            try {
                if (rs != null) rs.close();
                if (stmt != null) stmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + isModerator("admin"));
        System.out.println("Test 2: " + isModerator("moderator1"));
        System.out.println("Test 3: " + isModerator("user123"));
        System.out.println("Test 4: " + isModerator("guest"));
        System.out.println("Test 5: " + isModerator("nonexistent"));
    }
}
