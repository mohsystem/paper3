
import java.sql.*;
import java.util.*;
import java.util.regex.Pattern;

public class Lse143 {
    private static final String EMAIL_REGEX = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(EMAIL_REGEX);
    
    // Database connection configuration
    private static final String DB_URL = "jdbc:sqlite:mailing_list.db";
    
    public static String unsubscribe(String email) {
        // Input validation
        if (email == null || email.trim().isEmpty()) {
            return "Invalid email parameter";
        }
        
        email = email.trim();
        
        // Validate email format
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Invalid email format";
        }
        
        Connection conn = null;
        PreparedStatement checkStmt = null;
        PreparedStatement deleteStmt = null;
        ResultSet rs = null;
        
        try {
            // Establish database connection
            conn = DriverManager.getConnection(DB_URL);
            
            // Check if email exists in database
            String checkQuery = "SELECT email FROM subscribers WHERE email = ?";
            checkStmt = conn.prepareStatement(checkQuery);
            checkStmt.setString(1, email);
            rs = checkStmt.executeQuery();
            
            if (rs.next()) {
                // Email exists, delete the user
                String deleteQuery = "DELETE FROM subscribers WHERE email = ?";
                deleteStmt = conn.prepareStatement(deleteQuery);
                deleteStmt.setString(1, email);
                int rowsAffected = deleteStmt.executeUpdate();
                
                if (rowsAffected > 0) {
                    return "Successfully unsubscribed: " + email;
                } else {
                    return "Failed to unsubscribe";
                }
            } else {
                // Email not found in database
                return "User is not subscribed";
            }
            
        } catch (SQLException e) {
            return "Database error: " + e.getMessage();
        } finally {
            // Close resources
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
    
    // Helper method to initialize test database
    private static void initTestDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("CREATE TABLE IF NOT EXISTS subscribers (id INTEGER PRIMARY KEY AUTOINCREMENT, email TEXT UNIQUE NOT NULL)");
            stmt.execute("DELETE FROM subscribers");
            stmt.execute("INSERT INTO subscribers (email) VALUES ('test1@example.com')");
            stmt.execute("INSERT INTO subscribers (email) VALUES ('test2@example.com')");
            stmt.execute("INSERT INTO subscribers (email) VALUES ('user@domain.com')");
            
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Initialize test database
        initTestDatabase();
        
        // Test case 1: Valid unsubscribe
        System.out.println("Test 1: " + unsubscribe("test1@example.com"));
        
        // Test case 2: User not subscribed
        System.out.println("Test 2: " + unsubscribe("notfound@example.com"));
        
        // Test case 3: Another valid unsubscribe
        System.out.println("Test 3: " + unsubscribe("test2@example.com"));
        
        // Test case 4: Invalid email format
        System.out.println("Test 4: " + unsubscribe("invalid-email"));
        
        // Test case 5: Empty email
        System.out.println("Test 5: " + unsubscribe(""));
    }
}
