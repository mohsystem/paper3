
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Lse98 {
    
    public static String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        byte[] hashedPassword = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hashedPassword);
    }
    
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    public static boolean registerUser(String username, String password, Connection conn) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return false;
        }
        
        try {
            byte[] salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            String saltString = Base64.getEncoder().encodeToString(salt);
            
            String sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
            PreparedStatement pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.setString(3, saltString);
            
            int rowsAffected = pstmt.executeUpdate();
            pstmt.close();
            
            return rowsAffected > 0;
        } catch (Exception e) {
            System.err.println("Error registering user: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test cases (using in-memory database for demonstration)
            Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:");
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)");
            
            // Test case 1: Valid registration
            System.out.println("Test 1: " + registerUser("john_doe", "SecurePass123!", conn));
            
            // Test case 2: Another valid registration
            System.out.println("Test 2: " + registerUser("jane_smith", "MyP@ssw0rd", conn));
            
            // Test case 3: Empty username
            System.out.println("Test 3: " + registerUser("", "password123", conn));
            
            // Test case 4: Empty password
            System.out.println("Test 4: " + registerUser("user123", "", conn));
            
            // Test case 5: Null values
            System.out.println("Test 5: " + registerUser(null, null, conn));
            
            conn.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
