
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class Lse139 {
    
    public static boolean isUserModerator(String username) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        
        String dbUrl = System.getenv("DB_URL");
        String dbUser = System.getenv("DB_USER");
        String dbPassword = System.getenv("DB_PASSWORD");
        
        if (dbUrl == null || dbUser == null || dbPassword == null) {
            System.err.println("Database credentials not configured");
            return false;
        }
        
        String query = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
        
        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return rs.getBoolean("is_moderator");
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error occurred");
            return false;
        }
        
        return false;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - Valid moderator: " + isUserModerator("admin"));
        System.out.println("Test 2 - Valid non-moderator: " + isUserModerator("user123"));
        System.out.println("Test 3 - Non-existent user: " + isUserModerator("nonexistent"));
        System.out.println("Test 4 - Null username: " + isUserModerator(null));
        System.out.println("Test 5 - Empty username: " + isUserModerator(""));
    }
}
