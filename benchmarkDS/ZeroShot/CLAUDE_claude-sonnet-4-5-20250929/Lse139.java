
import java.sql.*;

public class Lse139 {
    public static boolean isUserModerator(String username) {
        String dbUrl = System.getenv("DB_URL");
        String dbUser = System.getenv("DB_USER");
        String dbPassword = System.getenv("DB_PASSWORD");
        
        if (dbUrl == null || dbUser == null || dbPassword == null) {
            System.err.println("Database credentials not found in environment variables");
            return false;
        }
        
        String query = "SELECT is_moderator FROM users WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return rs.getBoolean("is_moderator");
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
        
        return false;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + isUserModerator("admin_user"));
        System.out.println("Test 2: " + isUserModerator("moderator1"));
        System.out.println("Test 3: " + isUserModerator("regular_user"));
        System.out.println("Test 4: " + isUserModerator("test_mod"));
        System.out.println("Test 5: " + isUserModerator("guest_user"));
    }
}
