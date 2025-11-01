
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse91 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    public static class RegistrationResult {
        public boolean success;
        public String message;
        
        public RegistrationResult(boolean success, String message) {
            this.success = success;
            this.message = message;
        }
    }
    
    public static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "username TEXT UNIQUE NOT NULL," +
                        "password_hash TEXT NOT NULL," +
                        "salt TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static RegistrationResult registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty()) {
            return new RegistrationResult(false, "Username cannot be empty");
        }
        if (password == null || password.length() < 8) {
            return new RegistrationResult(false, "Password must be at least 8 characters");
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(
                 "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)")) {
            
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.setString(3, salt);
            pstmt.executeUpdate();
            
            return new RegistrationResult(true, "User registered successfully");
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return new RegistrationResult(false, "Username already exists");
            }
            return new RegistrationResult(false, "Registration failed: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        // Test case 1: Valid registration
        RegistrationResult result1 = registerUser("john_doe", "SecurePass123");
        System.out.println("Test 1: " + result1.message);
        
        // Test case 2: Duplicate username
        RegistrationResult result2 = registerUser("john_doe", "AnotherPass456");
        System.out.println("Test 2: " + result2.message);
        
        // Test case 3: Short password
        RegistrationResult result3 = registerUser("jane_doe", "short");
        System.out.println("Test 3: " + result3.message);
        
        // Test case 4: Empty username
        RegistrationResult result4 = registerUser("", "ValidPassword123");
        System.out.println("Test 4: " + result4.message);
        
        // Test case 5: Valid registration with special characters
        RegistrationResult result5 = registerUser("alice_smith", "P@ssw0rd!2023");
        System.out.println("Test 5: " + result5.message);
    }
}
