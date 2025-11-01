
import java.sql.*;
import java.util.*;

public class Lse143 {
    private static final String DB_URL = "jdbc:sqlite:mailing_list.db";
    
    public static String unsubscribe(String email) {
        if (email == null || email.trim().isEmpty()) {
            return "Invalid email address";
        }
        
        Connection conn = null;
        PreparedStatement selectStmt = null;
        PreparedStatement deleteStmt = null;
        ResultSet rs = null;
        
        try {
            // Create database connection
            conn = DriverManager.getConnection(DB_URL);
            
            // Check if email exists in database
            String selectQuery = "SELECT email FROM subscribers WHERE email = ?";
            selectStmt = conn.prepareStatement(selectQuery);
            selectStmt.setString(1, email);
            rs = selectStmt.executeQuery();
            
            if (rs.next()) {
                // Email exists, delete it
                String deleteQuery = "DELETE FROM subscribers WHERE email = ?";
                deleteStmt = conn.prepareStatement(deleteQuery);
                deleteStmt.setString(1, email);
                deleteStmt.executeUpdate();
                
                return "Successfully unsubscribed: " + email;
            } else {
                // Email not found
                return "User is not subscribed: " + email;
            }
            
        } catch (SQLException e) {
            return "Error processing unsubscribe request: " + e.getMessage();
        } finally {
            // Close resources
            try {
                if (rs != null) rs.close();
                if (selectStmt != null) selectStmt.close();
                if (deleteStmt != null) deleteStmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
    
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            // Create table if not exists
            String createTableSQL = "CREATE TABLE IF NOT EXISTS subscribers (" +
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                                   "email TEXT UNIQUE NOT NULL)";
            stmt.execute(createTableSQL);
            
            // Insert test data
            String insertSQL = "INSERT OR IGNORE INTO subscribers (email) VALUES (?)";
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            
            String[] testEmails = {
                "user1@example.com",
                "user2@example.com",
                "user3@example.com"
            };
            
            for (String email : testEmails) {
                pstmt.setString(1, email);
                pstmt.executeUpdate();
            }
            
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Initialize database with test data
        initializeDatabase();
        
        // Test case 1: Unsubscribe existing user
        System.out.println("Test 1: " + unsubscribe("user1@example.com"));
        
        // Test case 2: Try to unsubscribe same user again
        System.out.println("Test 2: " + unsubscribe("user1@example.com"));
        
        // Test case 3: Unsubscribe another existing user
        System.out.println("Test 3: " + unsubscribe("user2@example.com"));
        
        // Test case 4: Try to unsubscribe non-existent user
        System.out.println("Test 4: " + unsubscribe("nonexistent@example.com"));
        
        // Test case 5: Try with empty email
        System.out.println("Test 5: " + unsubscribe(""));
    }
}
