
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class Lse139 {
    
    /**
     * Checks if a user is a moderator by querying the database.
     * Uses environment variables for database credentials.
     * 
     * @param username The username to check
     * @return true if user is a moderator, false otherwise
     */
    public static boolean isUserModerator(String username) {
        // Retrieve database credentials from environment variables
        // Never hardcode credentials (Rules#4, Rules#5)
        String dbUrl = System.getenv("DB_URL");
        String dbUser = System.getenv("DB_USER");
        String dbPassword = System.getenv("DB_PASSWORD");
        
        // Validate that credentials are available (Rules#8)
        if (dbUrl == null || dbUser == null || dbPassword == null) {
            throw new IllegalStateException("Database credentials not found in environment variables");
        }
        
        // Validate input username (Rules#8)
        if (username == null || username.trim().isEmpty()) {
            throw new IllegalArgumentException("Username cannot be null or empty");
        }
        
        // Limit username length to prevent potential attacks (Rules#8)
        if (username.length() > 255) {
            throw new IllegalArgumentException("Username exceeds maximum length");
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Establish database connection with proper resource management (Rules#11)
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            
            // Use parameterized query to prevent SQL injection (Rules#1)
            // Assuming table structure: users(username, is_moderator)
            String query = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
            pstmt = conn.prepareStatement(query);
            
            // Set parameter safely (Rules#1, Rules#8)
            pstmt.setString(1, username);
            
            // Execute query
            rs = pstmt.executeQuery();
            
            // Check result
            if (rs.next()) {
                return rs.getBoolean("is_moderator");
            }
            
            // User not found, return false
            return false;
            
        } catch (SQLException e) {
            // Log error securely without exposing sensitive details (Rules#1)
            System.err.println("Database error occurred while checking moderator status");
            // In production, use proper logging framework
            return false;
            
        } finally {
            // Ensure all resources are properly closed (Rules#11)
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing database resources");
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases - In production, set these as actual environment variables
        // For testing purposes only
        System.setProperty("DB_URL", "jdbc:mysql://localhost:3306/testdb");
        System.setProperty("DB_USER", "testuser");
        System.setProperty("DB_PASSWORD", "testpass");
        
        // Update to use System properties for testing
        String dbUrl = System.getProperty("DB_URL");
        String dbUser = System.getProperty("DB_USER");
        String dbPassword = System.getProperty("DB_PASSWORD");
        
        // Test case 1: Valid moderator user
        System.out.println("Test 1 - Valid moderator: " + isUserModerator("admin_user"));
        
        // Test case 2: Valid non-moderator user
        System.out.println("Test 2 - Regular user: " + isUserModerator("regular_user"));
        
        // Test case 3: Non-existent user
        System.out.println("Test 3 - Non-existent user: " + isUserModerator("nonexistent"));
        
        // Test case 4: Empty username (should throw exception)
        try {
            isUserModerator("");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 4 - Empty username: Exception caught as expected");
        }
        
        // Test case 5: Null username (should throw exception)
        try {
            isUserModerator(null);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 - Null username: Exception caught as expected");
        }
    }
}
