
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;

public class Lse36 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Hash password using SHA-256
    private static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hashedBytes = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedBytes);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    // Login function with secure password verification
    public static boolean login(String username, String password) {
        // Input validation
        if (username == null || password == null || username.trim().isEmpty() || password.trim().isEmpty()) {
            return false;
        }
        
        // Use parameterized query to prevent SQL injection
        String query = "SELECT password_hash FROM users WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                String inputHash = hashPassword(password);
                
                // Secure password comparison
                if (storedHash.equals(inputHash)) {
                    System.out.println("Login successful. Redirecting to home page...");
                    return true;
                } else {
                    System.out.println("Incorrect password.");
                    return false;
                }
            } else {
                System.out.println("Username not found.");
                return false;
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    // Helper method to create test database and users
    private static void setupTestDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("DROP TABLE IF EXISTS users");
            stmt.execute("CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT UNIQUE, password_hash TEXT)");
            
            String insertQuery = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
            PreparedStatement pstmt = conn.prepareStatement(insertQuery);
            
            // Add test users
            String[][] testUsers = {
                {"admin", "admin123"},
                {"user1", "password1"},
                {"testuser", "test@123"},
                {"john_doe", "secure_pass"},
                {"alice", "alice2024"}
            };
            
            for (String[] user : testUsers) {
                pstmt.setString(1, user[0]);
                pstmt.setString(2, hashPassword(user[1]));
                pstmt.executeUpdate();
            }
            
            System.out.println("Test database setup complete.\\n");
        } catch (SQLException e) {
            System.err.println("Error setting up database: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        setupTestDatabase();
        
        System.out.println("=== Test Cases ===\\n");
        
        // Test case 1: Valid login
        System.out.println("Test 1 - Valid login:");
        login("admin", "admin123");
        System.out.println();
        
        // Test case 2: Wrong password
        System.out.println("Test 2 - Wrong password:");
        login("admin", "wrongpassword");
        System.out.println();
        
        // Test case 3: Non-existent user
        System.out.println("Test 3 - Non-existent user:");
        login("nonexistent", "password");
        System.out.println();
        
        // Test case 4: Empty credentials
        System.out.println("Test 4 - Empty credentials:");
        login("", "");
        System.out.println();
        
        // Test case 5: Null credentials
        System.out.println("Test 5 - Null credentials:");
        login(null, null);
    }
}
