
import java.sql.*;
import java.util.*;

public class Lse35 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Initialize database and create users table
    public static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "username TEXT NOT NULL UNIQUE," +
                        "password TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    // Add a user to database (for testing)
    public static void addUser(String username, String password) {
        String sql = "INSERT INTO users (username, password) VALUES (?, ?)";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            pstmt.setString(2, password);
            pstmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    // Login function
    public static String login(String username, String password) {
        String sql = "SELECT * FROM users WHERE username = ? AND password = ?";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            pstmt.setString(2, password);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                return "home.html"; // Redirect to home page
            } else {
                return "login.html"; // Redirect to login page
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return "login.html";
        }
    }
    
    public static void main(String[] args) {
        // Initialize database
        initDatabase();
        
        // Clear existing data and add test users
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            stmt.execute("DELETE FROM users");
        } catch (SQLException e) {
            e.printStackTrace();
        }
        
        addUser("admin", "admin123");
        addUser("user1", "pass123");
        addUser("john", "doe456");
        
        // Test cases
        System.out.println("Test Case 1: Valid credentials (admin/admin123)");
        System.out.println("Result: " + login("admin", "admin123"));
        System.out.println();
        
        System.out.println("Test Case 2: Valid credentials (user1/pass123)");
        System.out.println("Result: " + login("user1", "pass123"));
        System.out.println();
        
        System.out.println("Test Case 3: Invalid password");
        System.out.println("Result: " + login("admin", "wrongpass"));
        System.out.println();
        
        System.out.println("Test Case 4: Non-existent user");
        System.out.println("Result: " + login("nonuser", "password"));
        System.out.println();
        
        System.out.println("Test Case 5: Valid credentials (john/doe456)");
        System.out.println("Result: " + login("john", "doe456"));
    }
}
