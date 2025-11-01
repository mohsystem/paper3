
import java.sql.*;
import java.util.Scanner;

public class Lse38 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String authenticateUser(String username, String password) {
        try {
            Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            String query = "SELECT * FROM users WHERE username = ? AND password = ?";
            PreparedStatement pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            pstmt.setString(2, password);
            
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                conn.close();
                return "dashboard";
            } else {
                conn.close();
                return "login";
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return "error";
        }
    }
    
    public static void main(String[] args) {
        // Test cases (simulated - actual database connection would be required)
        System.out.println("Test Case 1:");
        System.out.println("Result: " + authenticateUser("admin", "admin123"));
        
        System.out.println("\\nTest Case 2:");
        System.out.println("Result: " + authenticateUser("user1", "pass123"));
        
        System.out.println("\\nTest Case 3:");
        System.out.println("Result: " + authenticateUser("wronguser", "wrongpass"));
        
        System.out.println("\\nTest Case 4:");
        System.out.println("Result: " + authenticateUser("", ""));
        
        System.out.println("\\nTest Case 5:");
        System.out.println("Result: " + authenticateUser("testuser", "testpass"));
    }
}
