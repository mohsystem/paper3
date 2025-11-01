
import java.sql.*;
import java.util.Scanner;

public class Lse142 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/emaildb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String unsubscribe(String email) {
        Connection conn = null;
        PreparedStatement checkStmt = null;
        PreparedStatement deleteStmt = null;
        ResultSet rs = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Check if email exists
            String checkQuery = "SELECT email FROM subscribers WHERE email = ?";
            checkStmt = conn.prepareStatement(checkQuery);
            checkStmt.setString(1, email);
            rs = checkStmt.executeQuery();
            
            if (rs.next()) {
                // Email exists, remove it
                String deleteQuery = "DELETE FROM subscribers WHERE email = ?";
                deleteStmt = conn.prepareStatement(deleteQuery);
                deleteStmt.setString(1, email);
                int rowsAffected = deleteStmt.executeUpdate();
                
                if (rowsAffected > 0) {
                    return "Successfully unsubscribed: " + email;
                } else {
                    return "Error occurred while unsubscribing";
                }
            } else {
                return "Email not found in database: " + email;
            }
        } catch (SQLException e) {
            return "Database error: " + e.getMessage();
        } finally {
            try {
                if (rs != null) rs.close();
                if (checkStmt != null) checkStmt.close();
                if (deleteStmt != null) deleteStmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(unsubscribe("user1@example.com"));
        
        System.out.println("\\nTest Case 2:");
        System.out.println(unsubscribe("user2@example.com"));
        
        System.out.println("\\nTest Case 3:");
        System.out.println(unsubscribe("notindb@example.com"));
        
        System.out.println("\\nTest Case 4:");
        System.out.println(unsubscribe("user3@example.com"));
        
        System.out.println("\\nTest Case 5:");
        System.out.println(unsubscribe("invalid@test.com"));
    }
}
