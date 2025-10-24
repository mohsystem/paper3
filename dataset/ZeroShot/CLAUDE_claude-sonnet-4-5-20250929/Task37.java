
import java.sql.*;
import java.util.Scanner;

public class Task37 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Method to retrieve user information securely using PreparedStatement
    public static String getUserInfo(String username) {
        if (username == null || username.trim().isEmpty()) {
            return "Invalid username";
        }
        
        StringBuilder result = new StringBuilder();
        String query = "SELECT id, username, email, created_date FROM users WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            // Use parameterized query to prevent SQL injection
            pstmt.setString(1, username);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    result.append("User ID: ").append(rs.getInt("id")).append("\\n");
                    result.append("Username: ").append(rs.getString("username")).append("\\n");
                    result.append("Email: ").append(rs.getString("email")).append("\\n");
                    result.append("Created Date: ").append(rs.getString("created_date"));
                } else {
                    result.append("User not found");
                }
            }
        } catch (SQLException e) {
            result.append("Database error: ").append(e.getMessage());
        }
        
        return result.toString();
    }
    
    // Helper method to initialize database with test data
    private static void initializeDatabase() {
        String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                "username TEXT NOT NULL UNIQUE, " +
                "email TEXT NOT NULL, " +
                "created_date TEXT NOT NULL)";
        
        String insertSQL = "INSERT OR IGNORE INTO users (username, email, created_date) VALUES (?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute(createTableSQL);
            
            try (PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
                String[][] testData = {
                    {"john_doe", "john@example.com", "2024-01-15"},
                    {"jane_smith", "jane@example.com", "2024-02-20"},
                    {"bob_wilson", "bob@example.com", "2024-03-10"},
                    {"alice_jones", "alice@example.com", "2024-04-05"},
                    {"charlie_brown", "charlie@example.com", "2024-05-12"}
                };
                
                for (String[] data : testData) {
                    pstmt.setString(1, data[0]);
                    pstmt.setString(2, data[1]);
                    pstmt.setString(3, data[2]);
                    pstmt.executeUpdate();
                }
            }
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Initialize database with test data
        initializeDatabase();
        
        System.out.println("=== Secure Database Query - Test Cases ===\\n");
        
        // Test Case 1: Valid username
        System.out.println("Test Case 1: Valid username 'john_doe'");
        System.out.println(getUserInfo("john_doe"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        // Test Case 2: Another valid username
        System.out.println("Test Case 2: Valid username 'jane_smith'");
        System.out.println(getUserInfo("jane_smith"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        // Test Case 3: Non-existent username
        System.out.println("Test Case 3: Non-existent username 'unknown_user'");
        System.out.println(getUserInfo("unknown_user"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        // Test Case 4: SQL injection attempt (secured by PreparedStatement)
        System.out.println("Test Case 4: SQL injection attempt");
        System.out.println(getUserInfo("' OR '1'='1"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        // Test Case 5: Empty username
        System.out.println("Test Case 5: Empty username");
        System.out.println(getUserInfo(""));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
    }
}
