
import java.sql.*;
import java.util.*;

public class Lse143 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Simulated database connection
    private static Connection getConnection() throws SQLException {
        return DriverManager.getConnection(DB_URL);
    }
    
    // Initialize database
    private static void initDatabase() {
        try (Connection conn = getConnection();
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS subscribers (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "email TEXT UNIQUE NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    // Main unsubscribe function
    public static String unsubscribeUser(String email) {
        // Input validation
        if (email == null || email.trim().isEmpty()) {
            return "Invalid email parameter";
        }
        
        // Sanitize email input
        email = email.trim().toLowerCase();
        
        // Validate email format
        if (!isValidEmail(email)) {
            return "Invalid email format";
        }
        
        try (Connection conn = getConnection()) {
            // Check if email exists using parameterized query
            String checkSql = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
            try (PreparedStatement checkStmt = conn.prepareStatement(checkSql)) {
                checkStmt.setString(1, email);
                ResultSet rs = checkStmt.executeQuery();
                
                if (rs.next() && rs.getInt(1) > 0) {
                    // Email exists, delete it
                    String deleteSql = "DELETE FROM subscribers WHERE email = ?";
                    try (PreparedStatement deleteStmt = conn.prepareStatement(deleteSql)) {
                        deleteStmt.setString(1, email);
                        int rowsAffected = deleteStmt.executeUpdate();
                        
                        if (rowsAffected > 0) {
                            return "Successfully unsubscribed: " + email;
                        } else {
                            return "Failed to unsubscribe";
                        }
                    }
                } else {
                    return "Email not subscribed: " + email;
                }
            }
        } catch (SQLException e) {
            return "Database error: " + e.getMessage();
        }
    }
    
    // Email validation
    private static boolean isValidEmail(String email) {
        String emailRegex = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
        return email != null && email.matches(emailRegex);
    }
    
    // Helper method to add subscriber for testing
    private static void addSubscriber(String email) {
        try (Connection conn = getConnection()) {
            String sql = "INSERT OR IGNORE INTO subscribers (email) VALUES (?)";
            try (PreparedStatement stmt = conn.prepareStatement(sql)) {
                stmt.setString(1, email.trim().toLowerCase());
                stmt.executeUpdate();
            }
        } catch (SQLException e) {
            System.err.println("Error adding subscriber: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initDatabase();
        
        // Test cases
        System.out.println("=== Test Case 1: Unsubscribe existing user ===");
        addSubscriber("user1@example.com");
        System.out.println(unsubscribeUser("user1@example.com"));
        
        System.out.println("\\n=== Test Case 2: Unsubscribe non-existing user ===");
        System.out.println(unsubscribeUser("nonexistent@example.com"));
        
        System.out.println("\\n=== Test Case 3: Invalid email format ===");
        System.out.println(unsubscribeUser("invalid-email"));
        
        System.out.println("\\n=== Test Case 4: Empty email ===");
        System.out.println(unsubscribeUser(""));
        
        System.out.println("\\n=== Test Case 5: Null email ===");
        System.out.println(unsubscribeUser(null));
    }
}
