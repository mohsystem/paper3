
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse91 {
    
    // Secure password hashing using SHA-256 with salt
    private static String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    // Generate random salt
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    // Validate username (alphanumeric and underscore only, 3-20 chars)
    private static boolean isValidUsername(String username) {
        return username != null && username.matches("^[a-zA-Z0-9_]{3,20}$");
    }
    
    // Validate password (at least 8 characters)
    private static boolean isValidPassword(String password) {
        return password != null && password.length() >= 8;
    }
    
    public static boolean registerUser(String username, String password) {
        // Input validation
        if (!isValidUsername(username)) {
            System.out.println("Invalid username format");
            return false;
        }
        
        if (!isValidPassword(password)) {
            System.out.println("Invalid password format");
            return false;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Use SQLite for demonstration (in production, use proper database)
            conn = DriverManager.getConnection("jdbc:sqlite:users.db");
            
            // Create table if not exists
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)");
            stmt.close();
            
            // Generate salt and hash password
            byte[] salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            String saltString = Base64.getEncoder().encodeToString(salt);
            
            // Use parameterized query to prevent SQL injection
            String sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.setString(3, saltString);
            
            pstmt.executeUpdate();
            System.out.println("User registered successfully: " + username);
            return true;
            
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                System.out.println("Username already exists");
            } else {
                System.out.println("Database error occurred");
            }
            return false;
        } catch (NoSuchAlgorithmException e) {
            System.out.println("Hashing algorithm error");
            return false;
        } finally {
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.out.println("Error closing resources");
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== User Registration System ===\\n");
        
        // Test case 1: Valid registration
        System.out.println("Test 1: Valid registration");
        registerUser("john_doe", "SecurePass123");
        
        // Test case 2: Duplicate username
        System.out.println("\\nTest 2: Duplicate username");
        registerUser("john_doe", "AnotherPass456");
        
        // Test case 3: Invalid username (too short)
        System.out.println("\\nTest 3: Invalid username");
        registerUser("ab", "ValidPass789");
        
        // Test case 4: Invalid password (too short)
        System.out.println("\\nTest 4: Invalid password");
        registerUser("jane_doe", "short");
        
        // Test case 5: Valid registration with special requirements
        System.out.println("\\nTest 5: Another valid registration");
        registerUser("alice_smith", "MySecurePassword2024");
    }
}
