
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;

public class Lse34 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Initialize database with test users
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                    "username TEXT UNIQUE NOT NULL," +
                    "password TEXT NOT NULL)";
            stmt.execute(createTableSQL);
            
            // Clear existing data
            stmt.execute("DELETE FROM users");
            
            // Insert test users with hashed passwords
            String[] testUsers = {
                "admin", "testuser", "john", "jane", "guest"
            };
            String[] testPasswords = {
                "admin123", "password", "john123", "jane456", "guest"
            };
            
            PreparedStatement pstmt = conn.prepareStatement(
                "INSERT OR IGNORE INTO users (username, password) VALUES (?, ?)");
            
            for (int i = 0; i < testUsers.length; i++) {
                pstmt.setString(1, testUsers[i]);
                pstmt.setString(2, hashPassword(testPasswords[i]));
                pstmt.executeUpdate();
            }
            
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    // Hash password using SHA-256
    private static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    // Authenticate user with prepared statements to prevent SQL injection
    public static String authenticateUser(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return "redirect:/login?error=empty";
        }
        
        String hashedPassword = hashPassword(password);
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(
                 "SELECT username FROM users WHERE username = ? AND password = ?")) {
            
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                return "redirect:/dashboard";
            } else {
                return "redirect:/login?error=invalid";
            }
            
        } catch (SQLException e) {
            System.err.println("Authentication error: " + e.getMessage());
            return "redirect:/login?error=system";
        }
    }
    
    public static void main(String[] args) {
        // Initialize database with test data
        initializeDatabase();
        
        System.out.println("=== Secure Login System Test Cases ===\\n");
        
        // Test Case 1: Valid credentials
        System.out.println("Test 1 - Valid login (admin/admin123):");
        String result1 = authenticateUser("admin", "admin123");
        System.out.println("Result: " + result1);
        System.out.println();
        
        // Test Case 2: Invalid password
        System.out.println("Test 2 - Invalid password (admin/wrongpass):");
        String result2 = authenticateUser("admin", "wrongpass");
        System.out.println("Result: " + result2);
        System.out.println();
        
        // Test Case 3: Non-existent user
        System.out.println("Test 3 - Non-existent user (hacker/hack123):");
        String result3 = authenticateUser("hacker", "hack123");
        System.out.println("Result: " + result3);
        System.out.println();
        
        // Test Case 4: Empty credentials
        System.out.println("Test 4 - Empty credentials:");
        String result4 = authenticateUser("", "");
        System.out.println("Result: " + result4);
        System.out.println();
        
        // Test Case 5: SQL Injection attempt
        System.out.println("Test 5 - SQL Injection attempt (admin' OR '1'='1/anything):");
        String result5 = authenticateUser("admin' OR '1'='1", "anything");
        System.out.println("Result: " + result5);
        System.out.println();
    }
}
