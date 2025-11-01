
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse99 {
    
    public static String hashPassword(String password) {
        try {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);
            
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes());
            
            byte[] combined = new byte[salt.length + hashedPassword.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(hashedPassword, 0, combined, salt.length, hashedPassword.length);
            
            return Base64.getEncoder().encodeToString(combined);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static boolean insertUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || 
            password == null || password.trim().isEmpty()) {
            return false;
        }
        
        String hashedPassword = hashPassword(password);
        String url = "jdbc:sqlite:users.db";
        
        try (Connection conn = DriverManager.getConnection(url)) {
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (" +
                                   "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                                   "username TEXT UNIQUE NOT NULL, " +
                                   "password TEXT NOT NULL)";
            try (Statement stmt = conn.createStatement()) {
                stmt.execute(createTableSQL);
            }
            
            String insertSQL = "INSERT INTO users (username, password) VALUES (?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
                pstmt.setString(1, username);
                pstmt.setString(2, hashedPassword);
                pstmt.executeUpdate();
                return true;
            }
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: " + insertUser("alice", "password123"));
        System.out.println("Test Case 2: " + insertUser("bob", "SecureP@ss456"));
        System.out.println("Test Case 3: " + insertUser("charlie", "Test!ng789"));
        System.out.println("Test Case 4: " + insertUser("", "password"));
        System.out.println("Test Case 5: " + insertUser("david", "MyP@ssw0rd"));
    }
}
