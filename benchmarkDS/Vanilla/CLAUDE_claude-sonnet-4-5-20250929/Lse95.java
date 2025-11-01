
import java.sql.*;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse95 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt.getBytes());
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
    
    public static boolean registerUser(String username, String password) {
        try {
            Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            String storedPassword = hashedPassword + ":" + salt;
            
            String sql = "INSERT INTO users (username, password) VALUES (?, ?)";
            PreparedStatement stmt = conn.prepareStatement(sql);
            stmt.setString(1, username);
            stmt.setString(2, storedPassword);
            
            int result = stmt.executeUpdate();
            
            stmt.close();
            conn.close();
            
            return result > 0;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static boolean verifyUser(String username, String password) {
        try {
            Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String sql = "SELECT password FROM users WHERE username = ?";
            PreparedStatement stmt = conn.prepareStatement(sql);
            stmt.setString(1, username);
            
            ResultSet rs = stmt.executeQuery();
            
            if (rs.next()) {
                String storedPassword = rs.getString("password");
                String[] parts = storedPassword.split(":");
                String storedHash = parts[0];
                String salt = parts[1];
                
                String hashedPassword = hashPassword(password, salt);
                
                rs.close();
                stmt.close();
                conn.close();
                
                return hashedPassword.equals(storedHash);
            }
            
            rs.close();
            stmt.close();
            conn.close();
            
            return false;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Register user1");
        System.out.println(registerUser("user1", "password123"));
        
        System.out.println("\\nTest Case 2: Register user2");
        System.out.println(registerUser("user2", "securePass456"));
        
        System.out.println("\\nTest Case 3: Verify user1 with correct password");
        System.out.println(verifyUser("user1", "password123"));
        
        System.out.println("\\nTest Case 4: Verify user1 with incorrect password");
        System.out.println(verifyUser("user1", "wrongpassword"));
        
        System.out.println("\\nTest Case 5: Verify non-existent user");
        System.out.println(verifyUser("user3", "password789"));
    }
}
