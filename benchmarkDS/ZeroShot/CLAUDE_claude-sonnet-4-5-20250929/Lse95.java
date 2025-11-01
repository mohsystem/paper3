
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Lse95 {
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
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt.getBytes());
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    public static RegistrationResult registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty()) {
            return new RegistrationResult(false, "Username cannot be empty");
        }
        if (password == null || password.length() < 6) {
            return new RegistrationResult(false, "Password must be at least 6 characters");
        }
        
        try {
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
            }
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return new RegistrationResult(false, "Username already exists");
            }
            return new RegistrationResult(false, "Registration failed: " + e.getMessage());
        } catch (NoSuchAlgorithmException e) {
            return new RegistrationResult(false, "Hashing algorithm error");
        }
    }
    
    public static boolean verifyLogin(String username, String password) {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(
                 "SELECT password_hash, salt FROM users WHERE username = ?")) {
            
            pstmt.setString(1, username);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                String salt = rs.getString("salt");
                String hashedPassword = hashPassword(password, salt);
                return storedHash.equals(hashedPassword);
            }
            return false;
        } catch (SQLException | NoSuchAlgorithmException e) {
            System.err.println("Login verification error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        initializeDatabase();
        
        System.out.println("=== Test Case 1: Valid Registration ===");
        RegistrationResult result1 = registerUser("john_doe", "password123");
        System.out.println(result1.message);
        
        System.out.println("\\n=== Test Case 2: Duplicate Username ===");
        RegistrationResult result2 = registerUser("john_doe", "anotherpass");
        System.out.println(result2.message);
        
        System.out.println("\\n=== Test Case 3: Empty Username ===");
        RegistrationResult result3 = registerUser("", "password123");
        System.out.println(result3.message);
        
        System.out.println("\\n=== Test Case 4: Short Password ===");
        RegistrationResult result4 = registerUser("jane_doe", "123");
        System.out.println(result4.message);
        
        System.out.println("\\n=== Test Case 5: Valid Login ===");
        registerUser("alice", "securepass");
        boolean loginSuccess = verifyLogin("alice", "securepass");
        System.out.println("Login successful: " + loginSuccess);
        
        boolean loginFail = verifyLogin("alice", "wrongpass");
        System.out.println("Login with wrong password: " + loginFail);
    }
}
