
import java.sql.*;
import java.util.regex.Pattern;

public class Lse142 {
    private static final String EMAIL_REGEX = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(EMAIL_REGEX);
    
    public static String unsubscribe(String email, Connection conn) {
        // Validate email format
        if (email == null || email.trim().isEmpty()) {
            return "Error: Email address cannot be empty.";
        }
        
        email = email.trim();
        
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Error: Invalid email address format.";
        }
        
        // Use prepared statement to prevent SQL injection
        String checkQuery = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
        String deleteQuery = "DELETE FROM subscribers WHERE email = ?";
        
        try {
            // Check if email exists
            try (PreparedStatement checkStmt = conn.prepareStatement(checkQuery)) {
                checkStmt.setString(1, email);
                ResultSet rs = checkStmt.executeQuery();
                
                if (rs.next() && rs.getInt(1) > 0) {
                    // Email exists, delete it
                    try (PreparedStatement deleteStmt = conn.prepareStatement(deleteQuery)) {
                        deleteStmt.setString(1, email);
                        int rowsAffected = deleteStmt.executeUpdate();
                        
                        if (rowsAffected > 0) {
                            return "Success: Email address has been unsubscribed.";
                        }
                    }
                } else {
                    return "Info: Email address not found in database.";
                }
            }
        } catch (SQLException e) {
            return "Error: Database operation failed.";
        }
        
        return "Error: Unsubscribe operation failed.";
    }
    
    public static void main(String[] args) {
        // Test cases with in-memory database
        try {
            // Setup in-memory database
            Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:");
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE subscribers (email TEXT PRIMARY KEY)");
            stmt.execute("INSERT INTO subscribers VALUES ('test1@example.com')");
            stmt.execute("INSERT INTO subscribers VALUES ('test2@example.com')");
            stmt.execute("INSERT INTO subscribers VALUES ('user@domain.com')");
            
            // Test Case 1: Valid email that exists
            System.out.println("Test 1: " + unsubscribe("test1@example.com", conn));
            
            // Test Case 2: Valid email that doesn't exist
            System.out.println("Test 2: " + unsubscribe("nonexistent@example.com", conn));
            
            // Test Case 3: Invalid email format
            System.out.println("Test 3: " + unsubscribe("invalid-email", conn));
            
            // Test Case 4: Empty email
            System.out.println("Test 4: " + unsubscribe("", conn));
            
            // Test Case 5: Valid email with whitespace
            System.out.println("Test 5: " + unsubscribe("  test2@example.com  ", conn));
            
            conn.close();
        } catch (Exception e) {
            System.out.println("Database setup error: " + e.getMessage());
        }
    }
}
