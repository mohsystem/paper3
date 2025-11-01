
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse37 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String login(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return "cant_login";
        }
        
        // Use prepared statement to prevent SQL injection
        String query = "SELECT * FROM users WHERE username = ? AND password = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            pstmt.setString(2, password);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return "success";
                } else {
                    return "cant_login";
                }
            }
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return "internal_server_error";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + login("admin", "admin123"));
        System.out.println("Test 2: " + login("user1", "pass123"));
        System.out.println("Test 3: " + login("invalid", "wrong"));
        System.out.println("Test 4: " + login("", ""));
        System.out.println("Test 5: " + login(null, null));
    }
}
