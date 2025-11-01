
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse93 {
    
    // Secure password hashing using SHA-256 with salt
    public static String hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    // Generate random salt
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    // Secure user registration using prepared statements to prevent SQL injection
    public static boolean registerUser(String username, String password, Connection conn) {
        // Input validation
        if (username == null || username.trim().isEmpty() || 
            password == null || password.length() < 8) {
            System.out.println("Invalid input: username cannot be empty and password must be at least 8 characters");
            return false;
        }
        
        // Sanitize username
        username = username.trim();
        if (!username.matches("^[a-zA-Z0-9_]{3,20}$")) {
            System.out.println("Invalid username format");
            return false;
        }
        
        try {
            // Generate salt and hash password
            byte[] salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            String saltString = Base64.getEncoder().encodeToString(salt);
            
            // Use parameterized query to prevent SQL injection
            String sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
            PreparedStatement pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.setString(3, saltString);
            
            int rowsAffected = pstmt.executeUpdate();
            pstmt.close();
            
            return rowsAffected > 0;
        } catch (SQLException e) {
            System.out.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    // Initialize database table
    public static void initializeDatabase(Connection conn) {
        try {
            Statement stmt = conn.createStatement();
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                                   "username TEXT UNIQUE NOT NULL, " +
                                   "password_hash TEXT NOT NULL, " +
                                   "salt TEXT NOT NULL)";
            stmt.execute(createTableSQL);
            stmt.close();
        } catch (SQLException e) {
            System.out.println("Error creating table: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Using SQLite for demonstration (in-memory database)
        String url = "jdbc:sqlite::memory:";
        
        try (Connection conn = DriverManager.getConnection(url)) {
            initializeDatabase(conn);
            
            // Test cases
            System.out.println("Test Case 1: Valid registration");
            System.out.println("Result: " + registerUser("testuser1", "SecurePass123", conn));
            
            System.out.println("\\nTest Case 2: Another valid registration");
            System.out.println("Result: " + registerUser("alice_99", "MyP@ssw0rd!", conn));
            
            System.out.println("\\nTest Case 3: Short password (should fail)");
            System.out.println("Result: " + registerUser("testuser2", "short", conn));
            
            System.out.println("\\nTest Case 4: Empty username (should fail)");
            System.out.println("Result: " + registerUser("", "ValidPass123", conn));
            
            System.out.println("\\nTest Case 5: Invalid username characters (should fail)");
            System.out.println("Result: " + registerUser("test@user!", "ValidPass123", conn));
            
        } catch (SQLException e) {
            System.out.println("Connection error: " + e.getMessage());
        }
    }
}
