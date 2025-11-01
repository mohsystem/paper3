
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse91 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes());
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }
    
    public static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "username TEXT UNIQUE NOT NULL," +
                        "password_hash TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.out.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public static boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || 
            password == null || password.isEmpty()) {
            return false;
        }
        
        String hashedPassword = hashPassword(password);
        String sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            System.out.println("Registration error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        // Test case 1: Valid registration
        System.out.println("Test 1 - Valid registration: " + 
            registerUser("john_doe", "password123"));
        
        // Test case 2: Another valid registration
        System.out.println("Test 2 - Valid registration: " + 
            registerUser("jane_smith", "securePass456"));
        
        // Test case 3: Duplicate username (should fail)
        System.out.println("Test 3 - Duplicate username: " + 
            registerUser("john_doe", "differentPassword"));
        
        // Test case 4: Empty username (should fail)
        System.out.println("Test 4 - Empty username: " + 
            registerUser("", "password789"));
        
        // Test case 5: Null password (should fail)
        System.out.println("Test 5 - Null password: " + 
            registerUser("bob_jones", null));
    }
}
