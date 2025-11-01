
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Base64;

public class Lse94 {
    private static final int SALT_LENGTH = 16; // 128-bit salt
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Generate cryptographically secure random salt
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }
    
    // Hash password with salt using SHA-256
    private static String hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            
            // Combine salt and hash for storage
            byte[] combined = new byte[salt.length + hashedPassword.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(hashedPassword, 0, combined, salt.length, hashedPassword.length);
            
            return Base64.getEncoder().encodeToString(combined);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        }
    }
    
    // Insert user into database with parameterized query to prevent SQL injection
    private static boolean insertUser(String username, String password) {
        // Validate username length and characters (alphanumeric only)
        if (username == null || username.length() < 3 || username.length() > 50 || 
            !username.matches("^[a-zA-Z0-9_]+$")) {
            System.err.println("Invalid username format");
            return false;
        }
        
        // Validate password strength (minimum 8 characters)
        if (password == null || password.length() < 8) {
            System.err.println("Password must be at least 8 characters");
            return false;
        }
        
        // Generate unique salt for this user
        byte[] salt = generateSalt();
        
        // Hash password with salt
        String hashedPassword = hashPassword(password, salt);
        
        // Use parameterized query to prevent SQL injection
        String sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            // Set parameters - prevents SQL injection
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            
            pstmt.executeUpdate();
            return true;
            
        } catch (SQLException e) {
            System.err.println("Database error occurred");
            return false;
        }
    }
    
    // Create users table if it doesn't exist
    private static void initializeDatabase() {
        String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                               "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                               "username TEXT UNIQUE NOT NULL, " +
                               "password_hash TEXT NOT NULL)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(createTableSQL)) {
            pstmt.executeUpdate();
        } catch (SQLException e) {
            System.err.println("Failed to initialize database");
        }
    }
    
    public static void main(String[] args) {
        // Initialize database
        initializeDatabase();
        
        // Test case 1: Valid user registration
        System.out.println("Test 1: " + (insertUser("alice123", "SecurePass123!") ? "PASS" : "FAIL"));
        
        // Test case 2: Valid user with different credentials
        System.out.println("Test 2: " + (insertUser("bob_user", "MyP@ssw0rd2024") ? "PASS" : "FAIL"));
        
        // Test case 3: Invalid username (too short)
        System.out.println("Test 3: " + (!insertUser("ab", "ValidPass123") ? "PASS" : "FAIL"));
        
        // Test case 4: Invalid password (too short)
        System.out.println("Test 4: " + (!insertUser("charlie", "short") ? "PASS" : "FAIL"));
        
        // Test case 5: Invalid username (special characters)
        System.out.println("Test 5: " + (!insertUser("user@name", "ValidPass123") ? "PASS" : "FAIL"));
    }
}
