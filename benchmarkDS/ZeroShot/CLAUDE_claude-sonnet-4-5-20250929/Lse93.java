
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Lse93 {
    
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
    
    public static boolean insertUser(String username, String password, Connection conn) {
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
            e.printStackTrace();
            return false;
        }
    }
    
    public static void main(String[] args) {
        try {
            // Note: This is a test setup using in-memory H2 database
            Class.forName("org.h2.Driver");
            Connection conn = DriverManager.getConnection("jdbc:h2:mem:testdb", "sa", "");
            
            // Create table
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE users (id INT AUTO_INCREMENT PRIMARY KEY, username VARCHAR(255) UNIQUE, password_hash VARCHAR(255), salt VARCHAR(255))");
            stmt.close();
            
            // Test case 1
            System.out.println("Test 1: " + insertUser("user1", "password123", conn));
            
            // Test case 2
            System.out.println("Test 2: " + insertUser("user2", "securePass456", conn));
            
            // Test case 3
            System.out.println("Test 3: " + insertUser("user3", "myP@ssw0rd", conn));
            
            // Test case 4
            System.out.println("Test 4: " + insertUser("user4", "testPassword789", conn));
            
            // Test case 5
            System.out.println("Test 5: " + insertUser("user5", "anotherSecure#Pass", conn));
            
            conn.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
