
import java.sql.*;
import java.util.Scanner;

public class Lse36 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String login(String username, String password) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            String query = "SELECT password FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedPassword = rs.getString("password");
                if (storedPassword.equals(password)) {
                    return "redirect:home";
                } else {
                    return "incorrect_password";
                }
            } else {
                return "user_not_found";
            }
        } catch (SQLException e) {
            return "error: " + e.getMessage();
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
        // Test cases
        System.out.println("Test Case 1: " + login("admin", "admin123"));
        System.out.println("Test Case 2: " + login("user1", "wrongpass"));
        System.out.println("Test Case 3: " + login("john_doe", "pass123"));
        System.out.println("Test Case 4: " + login("nonexistent", "password"));
        System.out.println("Test Case 5: " + login("testuser", "testpass"));
    }
}
