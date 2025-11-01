
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Lse99 {
    
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
            // Setup in-memory database for testing
            Connection conn = DriverManager.getConnection("jdbc:sqlite::memory:");
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)");
            
            // Test cases
            System.out.println("Test 1: " + insertUser("user1", "Password123!", conn));
            System.out.println("Test 2: " + insertUser("user2", "SecureP@ss456", conn));
            System.out.println("Test 3: " + insertUser("user3", "MySecret789#", conn));
            System.out.println("Test 4: " + insertUser("user4", "Test@1234", conn));
            System.out.println("Test 5: " + insertUser("user5", "Admin$Pass999", conn));
            
            conn.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
